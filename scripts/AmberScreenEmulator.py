#!/usr/bin/env python3

import pygame
import pygame.freetype
from PIL import Image, ImageFilter
import threading
import ctypes
import io
import os

KEYBOARD_INTERRUPTION = 1

# Mapping of characters to corresponding key codes
KEY_MAPPING = {
    'a': 0, 'b': 1, 'c': 2, 'd': 3, 'e': 4, 'f': 5, 'g': 6, 'h': 7, 'i': 8, 'j': 9, 'k': 10,
    'l': 11, 'm': 12, 'n': 13, 'o': 14, 'p': 15, 'q': 16, 'r': 17, 's': 18, 't': 19, 'u': 20,
    'v': 21, 'w': 22, 'x': 23, 'y': 24, 'z': 25, '0': 26, '1': 27, '2': 28, '3': 29, '4': 30,
    '5': 31, '6': 32, '7': 33, '8': 34, '9': 35, '!': 36, '@': 37, '#': 38, '$': 39, '%': 40,
    '^': 41, '&': 42, '*': 43, '(': 44, ')': 45, '_': 46, '-': 47, '+': 48, '=': 49, '<': 50,
    ',': 51, '.': 52, '>': 53, ':': 54, ';': 55, '"': 56, "'": 57, '|': 58, '\\': 59, '?': 60,
    '/': 61, ' ': 62, '`': 119, '~': 120
}

NUMPAD_MAPPING = {
    pygame.K_KP0: 26, pygame.K_KP1: 27, pygame.K_KP2: 28, pygame.K_KP3: 29, pygame.K_KP4: 30, pygame.K_KP5: 31,
    pygame.K_KP6: 32, pygame.K_KP7: 33, pygame.K_KP8: 34, pygame.K_KP9: 35,
    pygame.K_KP_PERIOD: 52, pygame.K_KP_DIVIDE: 61, pygame.K_KP_MULTIPLY: 43, pygame.K_KP_MINUS: 47,
    pygame.K_KP_PLUS: 48, pygame.K_KP_ENTER: 89, pygame.K_KP_EQUALS: 49
}

SHIFT_MAPPING = {
    pygame.K_1: '!', pygame.K_2: '@', pygame.K_3: '#', pygame.K_4: '$', pygame.K_5: '%', pygame.K_6: '^',
    pygame.K_7: '&', pygame.K_8: '*', pygame.K_9: '(', pygame.K_0: ')', pygame.K_MINUS: '_', pygame.K_EQUALS: '+',
    pygame.K_LEFTBRACKET: '{', pygame.K_RIGHTBRACKET: '}', pygame.K_SEMICOLON: ':', pygame.K_QUOTE: '"',
    pygame.K_COMMA: '<', pygame.K_PERIOD: '>', pygame.K_SLASH: '?', pygame.K_BACKSLASH: '|', pygame.K_BACKQUOTE: '~',
}

MOD_MAPPING = {
    pygame.KMOD_CTRL: {
        pygame.K_a: 63, pygame.K_b: 64, pygame.K_c: 65, pygame.K_d: 66, pygame.K_e: 67, pygame.K_f: 68, pygame.K_g: 69,
        pygame.K_h: 70, pygame.K_i: 71, pygame.K_j: 72, pygame.K_k: 73, pygame.K_l: 74, pygame.K_m: 75, pygame.K_n: 76,
        pygame.K_o: 77, pygame.K_p: 78, pygame.K_q: 79, pygame.K_r: 80, pygame.K_s: 81, pygame.K_t: 82, pygame.K_u: 83,
        pygame.K_v: 84, pygame.K_w: 85, pygame.K_x: 86, pygame.K_y: 87, pygame.K_z: 88
    },

    pygame.KMOD_SHIFT: {
        pygame.K_c: 114, pygame.K_v: 115, pygame.K_s: 116, pygame.K_a: 117, pygame.K_e: 118
    }
}

SPECIAL_KEY_MAPPING = {
    pygame.K_UP: 91, pygame.K_DOWN: 92, pygame.K_LEFT: 93, pygame.K_RIGHT: 94, pygame.K_PAGEUP: 95, pygame.K_PAGEDOWN: 96,
    pygame.K_HOME: 97, pygame.K_END: 98, pygame.K_ESCAPE: 99, pygame.K_DELETE: 100, pygame.K_RETURN: 89, pygame.K_TAB: 90,
    pygame.K_BACKSPACE: 121
}

def construct_keyboard_response_code(key_mod, key):
    # Normal NumLock state
    if key_mod & pygame.KMOD_NUM:
        if key in NUMPAD_MAPPING:
            return NUMPAD_MAPPING[key]

    if key_mod & pygame.KMOD_SHIFT:
        if key in SHIFT_MAPPING:
            return KEY_MAPPING[SHIFT_MAPPING[key]]

    # Handle special keys like arrow keys, delete, etc.
    if key in SPECIAL_KEY_MAPPING:
        return SPECIAL_KEY_MAPPING[key]

    # Handle CTRL + key combinations
    if key_mod & pygame.KMOD_CTRL:
        # Handle CTRL + SHIFT + key combinations
        if key_mod & pygame.KMOD_SHIFT:
            return MOD_MAPPING.get(pygame.KMOD_SHIFT, {}).get(key, -1)
        # Normal Ctrl + keys
        return MOD_MAPPING.get(pygame.KMOD_CTRL, {}).get(key, -1)

    # Normal text input
    if key in range(0, 256):
        key = chr(key).lower()
        return KEY_MAPPING[key]

    return -1  # Return -1 for failure to match any key

def turn_upper_case(text):
    return text.upper()

class AmberScreenEmulator:
    def __init__(self,
                 xxd_font_library_pah,
                 interruption_invocation_library,
                 interruption_handler_notifier_pid,
                 caption        = 'Amber Phosphor Screen Emulator',
                 _char_width    = 20,
                 _char_height   = 30,
                 _font_size     = 32,
                 _screen_width  = 1366,
                 _screen_height = 768,
                 _cols          = 68,
                 _rows          = 25,
                 _fps           = 30):

        self.notifier_pid = interruption_handler_notifier_pid

        # Load the shared library
        self.font_lib = ctypes.CDLL(xxd_font_library_pah)

        # Get the pointer to the font data and the size of the font
        self.font_lib.get_font.restype = ctypes.POINTER(ctypes.c_ubyte)
        self.font_lib.get_font_len.restype = ctypes.c_size_t

        # Retrieve the font data and its size
        self.font_data_ptr = self.font_lib.get_font()
        self.font_data_size = self.font_lib.get_font_len()

        # Convert the font data to a Python byte string
        self.font_data = ctypes.string_at(self.font_data_ptr, self.font_data_size)

        # Use io.BytesIO to wrap the binary data as a file-like object
        self.font_file = io.BytesIO(self.font_data)

        # Interruption library
        self.interruption_lib = ctypes.CDLL(interruption_invocation_library)
        self.interruption_lib.call_interruption_handler.argtypes = [
            ctypes.c_int,       # Handler PID
            ctypes.c_int,       # Interruption code
            ctypes.c_int,       # Interruption flags
            ctypes.c_void_p,    # Parameter starting address (void *)
            ctypes.c_uint,      # Parameter numbers
        ]
        self.interruption_lib.call_interruption_handler.restype = ctypes.c_int
        # self.interruption_lib.initialize_interruption_handler()

        # Initialize pygame
        pygame.init()
        pygame.font.init()

        # Screen settings
        self.screen_width = _screen_width
        self.screen_height = _screen_height
        self.cols = _cols
        self.rows = _rows
        self.char_width = _char_width
        self.char_height = _char_height

        # Font settings
        self.font_size = _font_size
        self.font = pygame.freetype.Font(self.font_file, self.font_size)

        self.amber_color = (255, 191, 0)
        self.bg_color = (16, 2, 0)

        # Decay settings
        self.decay_buffer = [['' for _ in range(self.cols)] for _ in range(self.rows)]
        self.decay_timer = [[0 for _ in range(self.cols)] for _ in range(self.rows)]
        self.active_decay_positions = []
        self.max_decay_time = 50
        self.min_brightness_factor = 0.6

        # Initialize screen and glow surface
        self.screen = pygame.display.set_mode((self.screen_width, self.screen_height))
        pygame.display.set_caption(caption)
        self.glow_surface = self.screen.copy()

        # Input stream
        self.input_stream = []

        # Threading event to manage the running state
        self.running_event = threading.Event()
        self.running_event.set()

        # Thread object to control the service loop thread
        self.service_thread = None

        # Create clock for FPS control
        self.clock = pygame.time.Clock()
        self.FPS = _fps  # Reduced FPS for better CPU efficiency

    def invoke_interruption(self, interruption_code, flags, parameter_list):
        IntArrayType = ctypes.c_int * len(parameter_list)
        int_array = IntArrayType(*parameter_list)
        void_ptr = ctypes.cast(int_array, ctypes.c_void_p)

        return self.interruption_lib.call_interruption_handler(
            self.notifier_pid,
            interruption_code,
            flags,
            void_ptr,
            len(parameter_list))

    def add_glow(self, image_surface, blur_radius=8):
        pil_string_image = pygame.image.tostring(image_surface, "RGB")
        pil_image = Image.frombytes("RGB", image_surface.get_size(), pil_string_image)
        pil_image = pil_image.filter(ImageFilter.GaussianBlur(blur_radius))
        return pygame.image.fromstring(pil_image.tobytes(), pil_image.size, "RGB")

    def display_char(self, row, col, char):
        if (row < 0 or row >= self.rows) or (col < 0 or col >= self.cols):
            return -1

        char = turn_upper_case(char)

        # Check if the character changed
        if self.decay_buffer[row][col] != char:
            self.decay_buffer[row][col] = char
            self.decay_timer[row][col] = self.max_decay_time
            if (row, col) not in self.active_decay_positions:
                self.active_decay_positions.append((row, col))

        return 0

    def update_decay(self):
        for (row, col) in self.active_decay_positions[:]:
            if self.decay_timer[row][col] > 0:
                self.decay_timer[row][col] -= 1

    def render_pixelated_text(self, text, color, scale_factor=1.5):
        small_surface = pygame.Surface((self.char_width, self.char_height))
        small_surface.fill(self.bg_color)

        # Check for bottom-aligned characters and adjust y_offset
        bottom_aligned_chars = ['.', '_', ',']
        y_offset = int(self.char_height * 0.6) if text in bottom_aligned_chars else 0

        # Render the text onto the small surface
        self.font.render_to(small_surface, (0, y_offset), text, color)

        # Perform scaling for pixelation effect
        pixelated_surface = pygame.transform.scale(
            small_surface,
            (int(self.char_width // scale_factor), int(self.char_height // scale_factor))
        )

        # Scale it back to fit the character cell
        final_surface = pygame.transform.scale(pixelated_surface, (self.char_width, self.char_height))
        return final_surface

    def decay_buffer_changed(self):
        return any(self.decay_timer[row][col] > 0 for row in range(self.rows) for col in range(self.cols))

    def service_loop(self):
        try:
            while self.running_event.is_set():
                screen_updated = False

                if self.decay_buffer_changed():  # Only render if decay buffer changes
                    self.screen.fill(self.bg_color)

                    for row in range(self.rows):
                        for col in range(self.cols):
                            if self.decay_buffer[row][col] != '':
                                brightness_factor = max(self.decay_timer[row][col] / self.max_decay_time, self.min_brightness_factor)
                                current_color = (
                                    int(self.amber_color[0] * brightness_factor),
                                    int(self.amber_color[1] * brightness_factor),
                                    int(self.amber_color[2] * brightness_factor)
                                )

                                pixelated_text_surface = self.render_pixelated_text(self.decay_buffer[row][col], current_color)
                                self.screen.blit(pixelated_text_surface, (col * self.char_width, row * self.char_height))

                    self.update_decay()
                    self.glow_surface = self.add_glow(self.screen)
                    self.screen.blit(self.glow_surface, (0, 0), special_flags=pygame.BLEND_ADD)

                    screen_updated = True

                if screen_updated:
                    pygame.display.flip()

                # Control frame rate with a more CPU-friendly wait
                pygame.time.wait(int(1000 / self.FPS))

                # Handle events
                self.handle_events()

        except Exception as err:
            print(f"An error occurred during the service loop: {err}")

        finally:
            # Clean up properly when the loop is stopped
            self.cleanup()
            self.running_event.clear()

    def handle_events(self):
        for event in pygame.event.get():

            if event.type == pygame.QUIT:
                self.running_event.clear()

            elif event.type == pygame.KEYDOWN:
                key = event.key
                mods = pygame.key.get_mods()

                respond_code = construct_keyboard_response_code(mods, int(key))
                # ignore unmapped keyboard code
                if respond_code == -1:
                    return
                # Handle key combination (use mods and key together)
                self.invoke_interruption(KEYBOARD_INTERRUPTION, 0, [respond_code])

    def cleanup(self):
        pygame.quit()

    def start_service(self):
        try:
            # Ensure no thread is already running
            if self.service_thread and self.service_thread.is_alive():
                print("Service is already running.")
                return -1

            self.service_thread = threading.Thread(target=self.service_loop)
            self.service_thread.start()
            return 0

        except Exception as err:
            print(f'Error when starting service ({err})!')
            return -1

    def stop_service(self):
        try:
            if not self.running_event.is_set():
                return 0

            # Signal the service loop to stop
            self.running_event.clear()

            # Wait for the service loop to finish
            self.join_service_loop()

            return 0
        except Exception as err:
            print(f'Error when stopping service ({err})!')
            return -1

    def join_service_loop(self):
        if self.service_thread and self.service_thread.is_alive():
            self.service_thread.join()

    def sleep(self, seconds):
        threading.Event().wait(seconds)

    def get_current_config(self):
        ret = [ [ "Character Width",    self.char_width     ],
                [ "Character Height",   self.char_height    ],
                [ "Font Size",          self.font_size      ],
                [ "Screen Width",       self.screen_width   ],
                [ "Screen Height",      self.screen_height  ],
                [ "Columns",            self.cols           ],
                [ "Rows",               self.rows           ],
                [ "FPS",                self.FPS            ]
        ]

        return ret

# Screen = AmberScreenEmulator(
#     "/tmp/build/libxxd_binary_content.so",
#     "/tmp/build/libsysdarft_event_vec.so",
#     os.getpid(),
#     "(Untitled)")
# # print("Return value from interruption: ", Screen.invoke_interruption(0, [114514, 1, 2, 3]))
# print(Screen.get_current_config())
# Screen.start_service()
# Screen.display_char(0, 0, '_')
# Screen.display_char(0, 1, '>')
# Screen.display_char(0, 2, 'S')
# Screen.display_char(0, 3, 'y')
# Screen.display_char(0, 4, 's')
# Screen.display_char(0, 5, 'd')
# Screen.display_char(0, 6, 'a')
# Screen.display_char(0, 7, 'r')
# Screen.display_char(0, 8, 'f')
# Screen.display_char(0, 9, 't')
# Screen.display_char(24, 67, '#')
# Screen.join_service_loop()
# Screen.stop_service()
