#!/usr/bin/env python3

import pygame
import pygame.freetype
from PIL import Image, ImageFilter
import threading
import ctypes
import io

KEYBOARD_INTERRUPTION = 1
KEYBOARD_INTERRUPTION_NORMAL_INPUT = 0
KEYBOARD_INTERRUPTION_MOD_INPUT = 1

def turn_upper_case(text):
    return text.upper()

class AmberScreenEmulator:
    def __init__(self,
                 xxd_font_library_pah,
                 interruption_invocation_library,
                 caption        = 'Amber Phosphor Screen Emulator',
                 _char_width    = 20,
                 _char_height   = 30,
                 _font_size     = 32,
                 _screen_width  = 1366,
                 _screen_height = 768,
                 _cols          = 68,
                 _rows          = 25,
                 _fps           = 30):

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
            ctypes.c_int,       # Interruption code
            ctypes.c_void_p,    # Parameter starting address (void *)
            ctypes.c_uint,      # Parameter numbers
        ]
        self.interruption_lib.restype = ctypes.c_int
        self.interruption_lib.initialize_interruption_handler()

        # Initialize pygame
        pygame.init()
        pygame.font.init()

        self.running_verification_flag = False

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

        # Start text input for pygame
        pygame.key.start_text_input()

        # Create clock for FPS control
        self.clock = pygame.time.Clock()
        self.FPS = _fps  # Reduced FPS for better CPU efficiency

    def invoke_interruption(self, interruption_code, parameter_list):
        IntArrayType = ctypes.c_int * len(parameter_list)
        int_array = IntArrayType(*parameter_list)
        void_ptr = ctypes.cast(int_array, ctypes.c_void_p)

        return self.interruption_lib.call_interruption_handler(
            interruption_code,
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
        self.running_verification_flag = True

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
            self.running_verification_flag = False
            self.running_event.clear()

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running_event.clear()
            elif event.type == pygame.TEXTINPUT:
                character = event.text[0]
                if ord(character) in range(0, 256):
                    self.invoke_interruption(KEYBOARD_INTERRUPTION,
                                             [KEYBOARD_INTERRUPTION_NORMAL_INPUT,
                                              int(ord(turn_upper_case(character)[0]))])
            elif event.type == pygame.KEYDOWN:
                mods = pygame.key.get_mods()
                key = event.key
                self.invoke_interruption(KEYBOARD_INTERRUPTION,
                                         [KEYBOARD_INTERRUPTION_MOD_INPUT, int(mods), int(key)])

    def cleanup(self):
        pygame.key.stop_text_input()
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
            pygame.event.post(pygame.event.Event(pygame.QUIT))

            # Signal the service loop to stop
            self.running_event.clear()

            # Wait for the service loop to finish
            if self.service_thread:
                self.service_thread.join()

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
# Screen.sleep(10)
# Screen.stop_service()
