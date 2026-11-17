#include "pch.h"
#include "UpdateCheck.h"
#include "CSPMOD.h"
#include "CspData.h"
#include "DUI/DuiCommon.h"
#include "resource.h"

#include <SDL3/SDL.h>
#include <json/json.h>
#include <objbase.h>
#include <wincrypt.h>
#include <winhttp.h>

#include <array>
#include <charconv>
#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "winhttp.lib")

namespace
{
constexpr wchar_t releasesPath[] = L"/repos/MMmmmoko/ClipStudioPaint-Smooth-Color-Adjustment/releases?per_page=20";
constexpr std::string_view assetPrefix =
    "https://github.com/MMmmmoko/ClipStudioPaint-Smooth-Color-Adjustment/releases/download/";
constexpr int checkTimeoutMs = 10000;
constexpr int checkOperationTimeoutMs = 2500;
constexpr int downloadTimeoutMs = 10 * 60 * 1000;
constexpr int downloadOperationTimeoutMs = 30000;
constexpr size_t maxReleaseResponse = 4 * 1024 * 1024;
constexpr uint64_t maxUpdateArchive = 128 * 1024 * 1024;

using InternetHandle = std::unique_ptr<void, decltype(&WinHttpCloseHandle)>;
using Handle = std::unique_ptr<void, decltype(&CloseHandle)>;
using Version = std::array<unsigned, 5>;
constexpr unsigned stable = 3;

struct Release
{
    Version version;
    std::string tag;
    std::string notes;
    std::string assetPath;
    std::string digest;
    uint64_t assetSize;
};

std::optional<Version> ParseVersion(std::string_view tag)
{
    if (tag.empty())
        return {};
    if (tag.front() == 'c')
    {
        const auto delimiter = tag.find_first_of("-_");
        if (delimiter == std::string_view::npos || tag.substr(delimiter + 1, 1) != "m")
            return {};
        tag.remove_prefix(delimiter + 2);
    }
    else if (tag.front() == 'v' || tag.front() == 'm')
    {
        tag.remove_prefix(1);
    }

    const auto readNumber = [&tag](unsigned& number) {
        if (tag.empty())
            return false;
        const auto result = std::from_chars(tag.data(), tag.data() + tag.size(), number);
        if (result.ec != std::errc{})
            return false;
        tag.remove_prefix(static_cast<size_t>(result.ptr - tag.data()));
        return true;
    };

    Version version{0, 0, 0, stable, 0};
    for (size_t component = 0; component < 3; ++component)
    {
        if (!readNumber(version[component]))
            return {};
        if (component < 2)
        {
            if (tag.empty() || tag.front() != '.')
                return {};
            tag.remove_prefix(1);
        }
    }
    if (tag.empty())
        return version;
    if (tag.front() != '-' && tag.front() != '_')
        return {};
    tag.remove_prefix(1);

    constexpr std::string_view stages[] = {"alpha", "beta", "rc"};
    for (unsigned stage = 0; stage < stable; ++stage)
    {
        const auto name = stages[stage];
        if (tag.substr(0, name.size()) != name)
            continue;
        version[3] = stage;
        tag.remove_prefix(name.size());
        if (tag.empty())
            return version;
        if (tag.front() == '.' || tag.front() == '-')
            tag.remove_prefix(1);
        return readNumber(version[4]) && tag.empty() ? std::optional<Version>(version) : std::nullopt;
    }
    return {};
}

std::string JsonString(const Json::Value& object, const char* key)
{
    const auto& value = object[key];
    return value.isString() ? value.asString() : std::string{};
}

std::optional<Release> FindUpdate(const Json::Value& releases, const Version& current)
{
    std::optional<Release> newest;
    if (!releases.isArray())
        return newest;
    for (const auto& item : releases)
    {
        if (!item.isObject() || item["draft"] == Json::Value(true))
            continue;
        const auto tag = JsonString(item, "tag_name");
        if (tag.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.-") !=
            std::string::npos)
            continue;
        const auto version = ParseVersion(tag);
        if (!version || *version <= current || (newest && *version <= newest->version) ||
            (current[3] == stable && ((*version)[3] != stable || item["prerelease"] == Json::Value(true))))
            continue;

        const auto& assets = item["assets"];
        if (!assets.isArray())
            continue;
        for (const auto& asset : assets)
        {
            if (!asset.isObject() || !asset["size"].isUInt64() || !asset["size"].asUInt64() ||
                asset["size"].asUInt64() > maxUpdateArchive)
                continue;
            const auto name = JsonString(asset, "name");
            const auto url = JsonString(asset, "browser_download_url");
            if (name.size() <= 11 || name.compare(0, 7, "CSPMOD_") != 0 ||
                name.compare(name.size() - 4, 4, ".zip") != 0 ||
                name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.-") != std::string::npos ||
                url != std::string(assetPrefix) + tag + "/" + name)
                continue;

            auto digest = JsonString(asset, "digest");
            if (!digest.empty())
            {
                if (digest.size() != 71 || digest.compare(0, 7, "sha256:") != 0 ||
                    digest.find_first_not_of("0123456789abcdefABCDEF", 7) != std::string::npos)
                    continue;
                digest.erase(0, 7);
            }
            newest = Release{*version, tag, JsonString(item, "body"),
                url.substr(sizeof("https://github.com") - 1), std::move(digest), asset["size"].asUInt64()};
            break;
        }
    }
    return newest;
}

// A marker per version avoids overwriting another process's skip choice or settings.
std::filesystem::path SkipMarker(const Version& version)
{
    std::unique_ptr<char, decltype(&SDL_free)> prefPath(SDL_GetPrefPath("CatSoft", "CSPMOD"), SDL_free);
    if (!prefPath)
        return {};

    std::wstring fileName = L"UpdateSkipped";
    for (const auto component : version)
        fileName += L"-" + std::to_wstring(component);
    return std::filesystem::u8path(prefPath.get()) / fileName;
}

template <class Receive, class Cancelled>
bool Get(const wchar_t* host, const wchar_t* path, const wchar_t* headers, int operationTimeout,
    int totalTimeout, bool allowRedirect, const Receive& receive, const Cancelled& cancelled)
{
    const auto deadline = GetTickCount64() + totalTimeout;
    InternetHandle session(WinHttpOpen(L"CSPMOD Update", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0), WinHttpCloseHandle);
    if (!session || !WinHttpSetTimeouts(session.get(), operationTimeout, operationTimeout,
        operationTimeout, operationTimeout))
        return false;
    InternetHandle connection(WinHttpConnect(session.get(), host, INTERNET_DEFAULT_HTTPS_PORT, 0),
        WinHttpCloseHandle);
    if (!connection)
        return false;
    InternetHandle request(WinHttpOpenRequest(connection.get(), L"GET", path, nullptr,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE), WinHttpCloseHandle);
    DWORD redirectPolicy = allowRedirect ? WINHTTP_OPTION_REDIRECT_POLICY_DISALLOW_HTTPS_TO_HTTP :
        WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
    DWORD status = 0;
    DWORD statusSize = sizeof(status);
    if (!request || !WinHttpSetOption(request.get(), WINHTTP_OPTION_REDIRECT_POLICY,
        &redirectPolicy, sizeof(redirectPolicy)) || cancelled() ||
        !WinHttpSendRequest(request.get(), headers, static_cast<DWORD>(-1L),
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        GetTickCount64() >= deadline || cancelled() ||
        !WinHttpReceiveResponse(request.get(), nullptr) ||
        !WinHttpQueryHeaders(request.get(), WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX) ||
        status != HTTP_STATUS_OK)
        return false;

    std::array<char, 32768> buffer;
    while (GetTickCount64() < deadline && !cancelled())
    {
        DWORD count = 0;
        if (!WinHttpReadData(request.get(), buffer.data(), static_cast<DWORD>(buffer.size()), &count) ||
            GetTickCount64() >= deadline || cancelled())
            return false;
        if (!count)
            return true;
        if (!receive(buffer.data(), count))
            return false;
    }
    return false;
}

std::optional<Release> CheckForUpdate()
{
    const auto current = ParseVersion(UpdateCheck::CurrentTag);
    if (!current)
        return {};

    std::string response;
    const auto append = [&response](const char* data, DWORD size) {
        if (size > maxReleaseResponse - response.size())
            return false;
        response.append(data, size);
        return true;
    };
    if (!Get(L"api.github.com", releasesPath,
        L"Accept: application/vnd.github+json\r\nX-GitHub-Api-Version: 2022-11-28\r\n",
        checkOperationTimeoutMs, checkTimeoutMs, false, append, [] { return false; }))
        return {};

    Json::CharReaderBuilder builder;
    builder["failIfExtra"] = true;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value releases;
    try
    {
        if (!reader->parse(response.data(), response.data() + response.size(), &releases, nullptr))
            return {};
        return FindUpdate(releases, *current);
    }
    catch (const Json::Exception&)
    {
        return {};
    }
}

struct UpdateJob
{
    std::wstring id;
    std::filesystem::path archive;
    Handle ready{nullptr, CloseHandle};
    Handle cancel{nullptr, CloseHandle};
    bool handedOff = false;

    ~UpdateJob()
    {
        if (!handedOff && !archive.empty())
        {
            DeleteFileW(archive.c_str());
            RemoveDirectoryW(archive.parent_path().c_str());
        }
    }
};

std::shared_ptr<UpdateJob> CreateJob()
{
    GUID id{};
    wchar_t name[40]{};
    wchar_t temp[MAX_PATH + 1]{};
    const DWORD length = GetTempPathW(ARRAYSIZE(temp), temp);
    if (!length || length >= ARRAYSIZE(temp) ||
        FAILED(CoCreateGuid(&id)) || !StringFromGUID2(id, name, ARRAYSIZE(name)))
        return {};
    auto job = std::make_shared<UpdateJob>();
    job->id = name;
    const auto folder = std::filesystem::path(temp) / (L"CSPMOD-update-" + job->id);
    if (!CreateDirectoryW(folder.c_str(), nullptr))
        return {};
    job->archive = folder / L"update.zip";
    job->ready.reset(CreateEventW(nullptr, TRUE, FALSE, (L"Local\\CSPMOD-ready-" + job->id).c_str()));
    job->cancel.reset(CreateEventW(nullptr, TRUE, FALSE, (L"Local\\CSPMOD-cancel-" + job->id).c_str()));
    if (!job->ready || !job->cancel)
        return {};
    return job;
}

template <class Report>
bool Download(const Release& release, const UpdateJob& job, const std::weak_ptr<ui::WeakFlag>& window,
    const Report& report)
{
    const std::wstring assetPath(release.assetPath.begin(), release.assetPath.end());
    const HANDLE file = CreateFileW(job.archive.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;

    uint64_t total = 0;
    int previousPercent = 0;
    const auto write = [&](const char* data, DWORD size) {
        DWORD written = 0;
        if (size > release.assetSize - total || !WriteFile(file, data, size, &written, nullptr) ||
            written != size)
            return false;
        total += written;
        const auto percent = static_cast<int>(total * 100 / release.assetSize);
        if (percent != previousPercent)
        {
            previousPercent = percent;
            report(percent);
        }
        return true;
    };
    const bool downloaded = Get(L"github.com", assetPath.c_str(), L"Accept: application/zip\r\n",
        downloadOperationTimeoutMs, downloadTimeoutMs, true, write, [&window] { return window.expired(); });
    const bool closed = CloseHandle(file) != FALSE;
    return downloaded && closed && total == release.assetSize && !window.expired();
}

std::wstring PowerShellString(std::wstring_view value)
{
    std::wstring quoted = L"'";
    for (wchar_t c : value)
    {
        quoted += c;
        if (c == L'\'' || c == L'\u2018' || c == L'\u2019' || c == L'\u201a' || c == L'\u201b')
            quoted += c;
    }
    return quoted + L"'";
}

enum class Preparation { Ready, Failed, BadPackage, Unwritable };

struct InstallerStrings
{
    std::wstring title;
    std::wstring installFailure;
    std::wstring restartFailure;
};

Preparation PrepareInstaller(UpdateJob& job, const Release& release, const InstallerStrings& strings,
    const std::weak_ptr<ui::WeakFlag>& window)
{
    if (window.expired())
        return Preparation::Failed;

    wchar_t executable[32768]{};
    const auto module = reinterpret_cast<HMODULE>(CSPMOD::GetDllModule());
    const auto length = GetModuleFileNameW(module, executable, ARRAYSIZE(executable));
    if (!length || length == ARRAYSIZE(executable))
        return Preparation::Failed;
    const auto directory = std::filesystem::path(executable).parent_path();
    const auto script = FindResourceW(module, MAKEINTRESOURCEW(IDR_UPDATE_INSTALLER), RT_RCDATA);
    const auto resource = script ? LoadResource(module, script) : nullptr;
    if (!resource)
        return Preparation::Failed;
    const auto* source = static_cast<const char*>(LockResource(resource));
    if (!source)
        return Preparation::Failed;
    const auto size = SizeofResource(module, script);
    std::wstring command = L"& {\n";
    for (DWORD index = 0; index < size; ++index)
        command += static_cast<wchar_t>(static_cast<unsigned char>(source[index]));
    command += L"\n} -HostPid " + std::to_wstring(GetCurrentProcessId()) +
        L" -InstallDirectory " + PowerShellString(directory.native()) +
        L" -ArchivePath " + PowerShellString(job.archive.native()) +
        L" -JobId " + PowerShellString(job.id) +
        L" -Digest " + PowerShellString(std::wstring(release.digest.begin(), release.digest.end())) +
        L" -DialogTitle " + PowerShellString(strings.title) +
        L" -FailureMessage " + PowerShellString(strings.installFailure) +
        L" -RestartFailureMessage " + PowerShellString(strings.restartFailure);

    DWORD count = 0;
    constexpr DWORD base64 = CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF;
    const DWORD bytes = static_cast<DWORD>(command.size() * sizeof(wchar_t));
    const auto* data = reinterpret_cast<const BYTE*>(command.data());
    if (!CryptBinaryToStringW(data, bytes, base64, nullptr, &count))
        return Preparation::Failed;
    std::wstring encoded(count, L'\0');
    if (!CryptBinaryToStringW(data, bytes, base64, encoded.data(), &count))
        return Preparation::Failed;
    encoded.resize(count);

    wchar_t systemPath[MAX_PATH]{};
    const auto systemLength = GetSystemDirectoryW(systemPath, ARRAYSIZE(systemPath));
    if (!systemLength || systemLength >= ARRAYSIZE(systemPath))
        return Preparation::Failed;
    const auto powershell = std::filesystem::path(systemPath) / L"WindowsPowerShell\\v1.0\\powershell.exe";
    std::wstring arguments = L"powershell.exe -NoLogo -NoProfile -NonInteractive -EncodedCommand " + encoded;
    if (arguments.size() >= 32767)
        return Preparation::Failed;
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(powershell.c_str(), arguments.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
        nullptr, systemPath, &startup, &process))
        return Preparation::Failed;
    CloseHandle(process.hThread);
    Handle installer(process.hProcess, CloseHandle);
    job.handedOff = true;

    const HANDLE events[] = {job.ready.get(), installer.get()};
    const auto deadline = GetTickCount64() + 60000;
    while (!window.expired() && GetTickCount64() < deadline)
    {
        const auto result = WaitForMultipleObjects(ARRAYSIZE(events), events, FALSE, 100);
        if (result == WAIT_OBJECT_0)
            return Preparation::Ready;
        if (result == WAIT_OBJECT_0 + 1)
        {
            job.handedOff = false;
            DWORD exitCode = 0;
            if (GetExitCodeProcess(installer.get(), &exitCode))
            {
                if (exitCode == 2)
                    return Preparation::BadPackage;
                if (exitCode == 3)
                    return Preparation::Unwritable;
            }
            return Preparation::Failed;
        }
        if (result != WAIT_TIMEOUT)
            break;
    }
    SetEvent(job.cancel.get());
    return Preparation::Failed;
}

class UpdateDialog : public ui::WindowImplBase
{
public:
    explicit UpdateDialog(const Release& release) : release_(release) {}

    DString GetSkinFolder() override { return L"cspHelper_default"; }
    DString GetSkinFile() override { return L"Update_Dlg.xml"; }

    void OnInitWindow() override
    {
        ui::WindowImplBase::OnInitWindow();
        auto& lang = ui::GlobalManager::Instance().Lang();
        auto* versions = static_cast<ui::Label*>(FindControl(L"versions"));
        auto* notes = static_cast<ui::RichEdit*>(FindControl(L"notes"));
        auto* close = FindControl(L"close");
        progress_ = static_cast<ui::Progress*>(FindControl(L"progress"));
        download_ = static_cast<ui::Button*>(FindControl(L"download"));
        skip_ = static_cast<ui::Button*>(FindControl(L"skip"));
        later_ = static_cast<ui::Button*>(FindControl(L"later"));
        status_ = static_cast<ui::Label*>(FindControl(L"status"));
        if (!versions || !notes || !later_ || !close || !download_ || !skip_ || !status_ || !progress_)
            return;

        versions->SetText(lang.GetStringViaID(L"STRID_UPDATE_CURRENT") + L" " +
            ui::StringConvert::UTF8ToWString(UpdateCheck::CurrentTag) + L"     " +
            lang.GetStringViaID(L"STRID_UPDATE_AVAILABLE") + L" " + ui::StringConvert::UTF8ToWString(release_.tag));

        notes->SetText(release_.notes.empty() ? lang.GetStringViaID(L"STRID_UPDATE_NO_NOTES") :
            ui::StringConvert::UTF8ToWString(release_.notes));
        notes->SetReadOnly(true);
        notes->SetSel(0, 0);

        download_->AttachClick([this](const ui::EventArgs&) { StartDownload(); return true; });
        skip_->AttachClick([this](const ui::EventArgs&) { Skip(); return true; });
        later_->AttachClick([this](const ui::EventArgs&) { CloseWnd(); return true; });
        close->AttachClick([this](const ui::EventArgs&) { CloseWnd(); return true; });
        ready_ = true;
    }

    bool IsReady() const { return ready_; }

private:
    void Skip()
    {
        const auto marker = SkipMarker(release_.version);
        const HANDLE file = marker.empty() ? INVALID_HANDLE_VALUE : CreateFileW(marker.c_str(),
            GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE)
        {
            status_->SetTextId(L"STRID_UPDATE_SKIP_FAILED");
            return;
        }
        CloseHandle(file);
        CloseWnd();
    }

    void StartDownload()
    {
        const auto job = CreateJob();
        if (!job)
        {
            status_->SetTextId(L"STRID_UPDATE_DOWNLOAD_FAILED");
            return;
        }

        progress_->SetVisible(true);
        SetProgress(0);
        download_->SetEnabled(false);
        skip_->SetEnabled(false);
        later_->SetTextId(L"STRID_UPDATE_CANCEL_DOWNLOAD");
        const auto window = GetWeakFlag();
        const auto progress = UiBind(&UpdateDialog::SetProgress, this, std::placeholders::_1);
        const auto finish = UiBind(&UpdateDialog::FinishPreparation, this, std::placeholders::_1,
            std::placeholders::_2, std::placeholders::_3);
        auto& lang = ui::GlobalManager::Instance().Lang();
        const InstallerStrings strings{
            lang.GetStringViaID(L"STRID_ABOUT_SOFTWARANAME"),
            lang.GetStringViaID(L"STRID_UPDATE_INSTALL_FAILED"),
            lang.GetStringViaID(L"STRID_UPDATE_RESTART_FAILED")
        };
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUser,
            [release = release_, job, strings, window, progress, finish] {
                const bool downloaded = Download(release, *job, window, [&progress](int percent) {
                    ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI,
                        [progress, percent] { progress(percent); });
                });
                const auto result = downloaded ? PrepareInstaller(*job, release, strings, window) :
                    Preparation::Failed;
                ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, [finish, result, job, downloaded] {
                    if (finish.Expired())
                        SetEvent(job->cancel.get());
                    else
                        finish(result, job, downloaded);
                });
            });
    }

    void SetProgress(int percent)
    {
        progress_->SetValue(percent);
        status_->SetText(percent == 100 ?
            ui::GlobalManager::Instance().Lang().GetStringViaID(L"STRID_UPDATE_PREPARING") :
            ui::GlobalManager::Instance().Lang().GetStringViaID(L"STRID_UPDATE_DOWNLOADING") +
                L" " + std::to_wstring(percent) + L"%");
    }

    void ShowFailure(const wchar_t* message)
    {
        status_->SetTextId(message);
        progress_->SetVisible(false);
        download_->SetEnabled(true);
        skip_->SetEnabled(true);
        later_->SetTextId(L"STRID_UPDATE_LATER");
    }

    void FinishPreparation(Preparation result, const std::shared_ptr<UpdateJob>& job, bool downloaded)
    {
        if (result != Preparation::Ready)
        {
            const auto message = !downloaded ? L"STRID_UPDATE_DOWNLOAD_FAILED" :
                result == Preparation::BadPackage ? L"STRID_UPDATE_BAD_PACKAGE" :
                result == Preparation::Unwritable ? L"STRID_UPDATE_UNWRITABLE" : L"STRID_UPDATE_PREPARE_FAILED";
            ShowFailure(message);
            return;
        }

        const HWND owner = GetWindow(NativeWnd()->GetHWND(), GW_OWNER);
        DWORD processId = 0;
        GetWindowThreadProcessId(owner, &processId);
        if (processId != GetCurrentProcessId() || !::IsWindowEnabled(owner))
        {
            SetEvent(job->cancel.get());
            ShowFailure(L"STRID_UPDATE_CLOSE_CANCELLED");
            return;
        }

        status_->SetTextId(L"STRID_UPDATE_CLOSING");
        later_->SetEnabled(false);
        const auto window = GetWeakFlag();
        SendMessageW(owner, WM_CLOSE, 0, 0);
        if (::IsWindow(owner))
        {
            SetEvent(job->cancel.get());
            if (!window.expired())
            {
                later_->SetEnabled(true);
                ShowFailure(L"STRID_UPDATE_CLOSE_CANCELLED");
            }
        }
    }

    Release release_;
    ui::Button* download_ = nullptr;
    ui::Button* skip_ = nullptr;
    ui::Button* later_ = nullptr;
    ui::Label* status_ = nullptr;
    ui::Progress* progress_ = nullptr;
    bool ready_ = false;
};

void ShowWhenReady(const std::shared_ptr<const Release>& release, ULONGLONG deadline)
{
    const auto owner = reinterpret_cast<HWND>(CspData::GetNativeWindowHandle());
    if (!IsWindowVisible(owner) || !IsWindowEnabled(owner))
    {
        if (GetTickCount64() < deadline)
            ui::GlobalManager::Instance().Thread().PostDelayedTask(ui::kThreadUI,
                [release, deadline] { ShowWhenReady(release, deadline); }, 500);
        return;
    }
    const auto marker = SkipMarker(release->version);
    if (!marker.empty() && GetFileAttributesW(marker.c_str()) != INVALID_FILE_ATTRIBUTES)
        return;

    auto* dialog = new UpdateDialog(*release);
    auto& lang = ui::GlobalManager::Instance().Lang();
    if (!dialog->CreateWnd(nullptr, ui::WindowCreateParam(lang.GetStringViaID(L"STRID_ABOUT_SOFTWARANAME"))))
    {
        delete dialog;
        return;
    }
    dialog->PostQuitMsgWhenClosed(false);
    if (!dialog->IsReady())
    {
        dialog->CloseWnd();
        return;
    }
    SetWindowLongPtrW(dialog->NativeWnd()->GetHWND(), GWLP_HWNDPARENT, reinterpret_cast<LONG_PTR>(owner));
    dialog->CenterWindow();
    dialog->ShowWindow(ui::kSW_SHOW_NA);
}
}

void UpdateCheck::Start()
{
    ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUser, [] {
        auto release = CheckForUpdate();
        if (release)
            ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI,
                [update = std::make_shared<Release>(std::move(*release))] {
                    ShowWhenReady(update, GetTickCount64() + 60000);
                });
    });
}
