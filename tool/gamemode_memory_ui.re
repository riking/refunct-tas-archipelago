

fn create_memory_options_menu() {
    let elements = List::new();

    elements.push(UiElement::FloatInput(FloatInput {
        label: Text { text: "Number of buttons to hit (1 - 18)" }, input: f"{MEMORY_SETTINGS.number_of_buttons}",
        onclick: fn(input: string) {}, onchange: fn(input: string) {
            match input.parse_int() { Result::Ok(value) => if 1 <= value && value <= 18 { MEMORY_SETTINGS.number_of_buttons = value; }, Result::Err(e) => {}, }
        },
    }));
    
    elements.push(UiElement::Button(UiButton {
        label: Text { text: "Back" },
        onclick: fn(label: Text) { leave_ui();leave_ui();leave_ui(); enter_ui(create_memory_menu()); },
    }));

    enter_ui(Ui::new("Memory Options", elements));
}

fn create_memory_menu() -> Ui {
    let elements = List::new();

    elements.push(UiElement::Button(UiButton {
        label: Text { text: "Choose custom options" },
        onclick: fn(label: Text) {
            create_memory_options_menu();
        },
    }));

    elements.push(UiElement::Button(UiButton {
        label: Text { text: "Start!" },
        onclick: fn(label: Text) {
            memory_start_select();
            leave_ui();leave_ui();leave_ui();leave_ui();
        },
    }));

    elements.push(UiElement::Button(UiButton {
        label: Text { text: "Back" },
        onclick: fn(label: Text) { leave_ui(); },
    }));

    Ui::new("Offline Memory", elements)
}

fn memory_start_select() {
    Tas::set_level(30);
    add_component(MEMORY_COMPONENT);
}

fn memory_hud_text(text: string) -> string {
    if SETTINGS.archipelago_display_style != ArchipelagoDisplayStyle::Classic { return text; }

    let mut hud_text = "";
    for line in memory_get_status_text_lines() {
        let text = line.text;
        hud_text = f"{hud_text}{text}";
    }
    hud_text
}

fn memory_hub_color_coded() {
    let viewport = Tas::get_viewport_size();
    let w = viewport.width.to_float();
    let h = viewport.height.to_float();

    if SETTINGS.archipelago_display_style != ArchipelagoDisplayStyle::ColorCoded { return; }

    let lines = memory_get_status_text_lines();
    let anchor = SETTINGS.archipelago_display_position;
    match anchor {
        Anchor::TopLeft => {
            // Make sure we don't overlap with the menu
            let text_y = match UI_STACK.last() {
                Option::Some(ui) => {
                    let size = Tas::get_text_size("x", SETTINGS.ui_scale);
                    let line_height = size.height;

                    let elements = ui.elements;
                    line_height * (elements.len().to_float() + 1.5)
                },
                Option::None => 0.0,
            };

            ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, 5.0, text_y, anchor, 5.0);
        },
        Anchor::TopCenter    => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w/2.0, 5.0, anchor, 5.0); },
        Anchor::TopRight     => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w, 5.0, anchor, 5.0); },
        Anchor::CenterRight  => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w, h/2.0, anchor, 5.0); },
        Anchor::BottomRight  => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w, h, anchor, 5.0); },
        Anchor::BottomCenter => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w/2.0, h, anchor, 5.0); },
        Anchor::BottomLeft   => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, 5.0, h, anchor, 5.0); },
        Anchor::CenterLeft   => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, 5.0, h/2.0, anchor, 5.0); },

        pos => panic(f"unknown/invalid archipelago display position: {pos}"),
    };

    if MEMORY_STATE.started == 0 {
        let anchor2 = Anchor::BottomishCenter;
        let lines2 = List::new();
        lines2.push(ColorfulText { text: "Press New Game from the Refunct menu to start.\n", color: COLOR_WHITE });
        lines2.push(ColorfulText { text: "You can change gamemode in the 'm' menu.", color: COLOR_WHITE });
        ap_draw_colorful_text(lines2, AP_COLOR_GRAY_BG, w/2.0, 4.0*h/5.0, anchor2, 5.0);
    }
    
}

fn memory_get_status_text_lines() -> List<ColorfulText> {  
    let lines = List::new();

    if INPUT_MODE_IS_UI_ONLY {
        List::extend(lines, List::of(
            ColorfulText { text: "Movement input disabled\n", color: AP_COLOR_RED },
            ColorfulText { text: "Press F1 to enable movement input", color: AP_COLOR_RED },
            ColorfulText { text: "\n\n", color: AP_COLOR_RED },
        ));
    }

    List::extend(lines, match MEMORY_STATE.started {
        0 => List::of(
            ColorfulText { text: "Memory!\n", color: COLOR_WHITE },
            ColorfulText { text: "Press new game (in Refunct menu).\n", color: AP_COLOR_CYAN },
        ),
        _ => List::of(
            ColorfulText { text: f"\nCubes collected: {MEMORY_STATE.buttons_pressed} / {MEMORY_SETTINGS.number_of_buttons}", color: COLOR_WHITE },
        )
    });
    lines
}