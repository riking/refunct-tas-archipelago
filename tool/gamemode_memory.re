
struct MemoryState {
    started: int,
    possible_buttons: List<int>,
    activated_buttons: List<int>,
    buttons_pressed: int,
}

fn fresh_memory_state() -> MemoryState {
    MemoryState {
        started: 0,
        possible_buttons: List::new(),
        activated_buttons: List::new(),
        buttons_pressed: 0,
    }
}

struct MemorySettings {
    number_of_buttons: int,
}

fn default_memory_settings() -> MemorySettings {
    MemorySettings {
        number_of_buttons: 3,
    }
}

static mut MEMORY_STATE = fresh_memory_state();
static mut MEMORY_SETTINGS = default_memory_settings();

static mut MEMORY_COMPONENT = Component {
    id: MEMORY_COMPONENT_ID,
    conflicts_with: List::of(MEMORY_COMPONENT_ID, MAP_EDITOR_COMPONENT_ID, ARCHIPELAGO_COMPONENT_ID, MEMORY_COMPONENT_ID, MULTIPLAYER_COMPONENT_ID, NEW_GAME_100_PERCENT_COMPONENT_ID, NEW_GAME_ALL_BUTTONS_COMPONENT_ID, NEW_GAME_NGG_COMPONENT_ID, PRACTICE_COMPONENT_ID, RANDOMIZER_COMPONENT_ID, TAS_COMPONENT_ID, WINDSCREEN_WIPERS_COMPONENT_ID, ARCHIPELAGO_DISCONNECTED_INFO_COMPONENT_ID),
    tick_mode: TickMode::DontCare,
    requested_delta_time: Option::None,
    error_message: "",
    on_tick: fn() {},
    on_yield: fn() {},
    draw_hud_text: memory_hud_text,
    draw_hud_always: memory_hub_color_coded,
    on_new_game: fn() {
        Tas::set_goal_animation_should_play(true); // so it works for minigames :3
        Tas::enable_all_buttons();

        Tas::destroy_cubes(true, true);
        Tas::destroy_platforms(true, 2);
        Tas::destroy_spawners();

        Tas::abilities_set_swim(true);
        MEMORY_STATE.started = 1;
    },
    on_level_change: fn(old: int, new: int) {
        Tas::set_level(30);
    },
    on_buttons_change: fn(old: int, new: int) {},
    on_cubes_change: fn(old: int, new: int) {},
    on_platforms_change: fn(old: int, new: int) {},
    on_reset: fn(old: int, new: int) {},
    on_element_pressed: fn(index: ElementIndex) {
        if MEMORY_STATE.started == 0 {
            return;
        }
        if MEMORY_STATE.started == 1 {
            memory_start();
            return;
        }   
        if index.element_type == ElementType::Button {
            if index.cluster_index + 1 == 31 {
                for id in MEMORY_STATE.activated_buttons {
                    Tas::enable_button(id / 10 - 1, (id % 10) - 1, Color { red: 0., green: 1., blue: 0., alpha: 1. });
                }
                Tas::trigger_goal_animation();
                return;
            }
            let num = (index.cluster_index + 1) * 10 + index.element_index + 1;
            if MEMORY_STATE.activated_buttons.contains(num) {
                MEMORY_STATE.buttons_pressed += 1;
            } else {
                MEMORY_STATE.buttons_pressed -= 1000;
            }
        }
        if index.element_type == ElementType::Platform && MEMORY_STATE.started == 2 {
            MEMORY_STATE.started = 3;
            for id in MEMORY_STATE.possible_buttons {
                Tas::enable_button(id / 10 - 1, (id % 10) - 1, Color { red: 1., green: 0., blue: 0., alpha: 1. });
            }
        }

    },
    on_element_released: fn(index: ElementIndex) {},
    on_key_down: fn(key: KeyCode, is_repeat: bool) {},
    on_key_down_always: fn(key: KeyCode, is_repeat: bool) {},
    on_key_up: fn(key: KeyCode) {},
    on_key_up_always: fn(key: KeyCode) {},
    on_key_char: fn(c: string) {},
    on_key_char_always: fn(c: string) {},
    on_mouse_move: fn(x: int, y: int) {},
    on_component_enter: fn() {},
    on_component_exit: fn() { MEMORY_STATE = fresh_memory_state(); },
    on_resolution_change: fn() {},
    on_menu_open: fn() {},
};

fn memory_start(){
    Tas::set_goal_animation_should_play(true);
    Tas::abilities_set_swim(true);
    Tas::abilities_set_wall_jump(2, false);
    Tas::abilities_set_ledge_grab(true);
    Tas::abilities_set_jump_pads(true);
    Tas::abilities_set_pipes(true);
    Tas::abilities_set_lifts(true);
    Tas::deactivate_all_buttons();
    collect_all_vanilla_cubes();
    MEMORY_STATE.buttons_pressed = 0;
    Tas::set_location(Location { x: 2629.0479, y: -2249.6926, z: 1764.3643 });
    MEMORY_STATE.possible_buttons = List::of(11,21,31,41,51,61,71,72,81,91,101,102,111,121,131,141,151,161,171,181,182,191,201,211,221,231,241,251,261,262,263,271,281,282,291,301);
    MEMORY_STATE.activated_buttons = List::of(21, 51, 282);
    Tas::enable_button(30, 0, Color { red: 1., green: 0., blue: 0., alpha: 1. });
    MEMORY_STATE.started = 2;
}
