			if (L"TableGrid" == controlName)return new ui::Box(window,new TableGridLayout());
			if (L"NumEdit" == controlName)return new NumEdit(window);
			if (L"HexColorEdit" == controlName)return new HexColorEdit(window);
			if (L"LoopSlider" == controlName)return new LoopSlider(window);
			if (L"PowSlider" == controlName)return new PowSlider(window);