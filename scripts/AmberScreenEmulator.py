import pygame
import pygame.freetype
from PIL import Image, ImageFilter
import threading
import time
import ctypes
import io

def turn_upper_case(text):
    return text.upper()

class AmberScreenEmulator:
    def __init__(self,
                 library_pah,
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
        self.font_lib = ctypes.CDLL(library_pah)

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

    def add_glow(self, image_surface, blur_radius=8):
        pil_string_image = pygame.image.tostring(image_surface, "RGB")
        pil_image = Image.frombytes("RGB", image_surface.get_size(), pil_string_image)
        pil_image = pil_image.filter(ImageFilter.GaussianBlur(blur_radius))
        return pygame.image.fromstring(pil_image.tobytes(), pil_image.size, "RGB")

    def display_char(self, row, col, char):
        char = turn_upper_case(char)

        # Check if the character changed
        if self.decay_buffer[row][col] != char:
            self.decay_buffer[row][col] = char
            self.decay_timer[row][col] = self.max_decay_time
            if (row, col) not in self.active_decay_positions:
                self.active_decay_positions.append((row, col))

        # Starting x and y positions for the first character
        x_pos = col * self.char_width
        y_pos = row * self.char_height

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
            # Clean up properly when loop is stopped
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
                    self.input_stream += ["Input", [turn_upper_case(character), ""]]
            elif event.type == pygame.KEYDOWN:
                self.process_special_keys(event)

    def process_special_keys(self, event):
        mods = pygame.key.get_mods()
        if mods & pygame.KMOD_CTRL:
            if not (event.key == pygame.K_LCTRL or event.key == pygame.K_RCTRL):
                self.input_stream += ["Control", ["Control", pygame.key.name(event.key)]]

        special_keys = {
            pygame.K_TAB: ["Input", ["\t", ""]],
            pygame.K_RETURN: ["Control", ["Enter", ""]],
            pygame.K_BACKSPACE: ["Control", ["Backspace", ""]],
            pygame.K_UP: ["Control", ["Up", ""]],
            pygame.K_DOWN: ["Control", ["Down", ""]],
            pygame.K_LEFT: ["Control", ["Left", ""]],
            pygame.K_RIGHT: ["Control", ["Right", ""]],
            pygame.K_PAGEUP: ["Control", ["PageUp", ""]],
            pygame.K_PAGEDOWN: ["Control", ["PageDown", ""]],
            pygame.K_HOME: ["Control", ["Home", ""]],
            pygame.K_END: ["Control", ["End", ""]],
            pygame.K_INSERT: ["Control", ["Insert", ""]],
            pygame.K_DELETE: ["Control", ["Delete", ""]],
            pygame.K_BREAK: ["Control", ["ExternalSyscall", ""]],
            pygame.K_PAUSE: ["Control", ["ExternalSyscall", ""]]
        }

        if event.key in special_keys:
            self.input_stream += special_keys[event.key]

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

    def query_input_stream(self):
        return self.input_stream

    def input_stream_pop_front(self):
        if self.input_stream:
            self.input_stream.pop(0)

    def join_service_loop(self):
        if self.service_thread and self.service_thread.is_alive():
            self.service_thread.join()

    def sleep(self, seconds):
        time.sleep(seconds)

Screen = AmberScreenEmulator("/tmp/build/libxxd_binary_content.so", "(Untitled)")
Screen.start_service()
Screen.display_char(0, 0, '_')
Screen.display_char(0, 1, '>')
Screen.display_char(0, 2, 'S')
Screen.display_char(0, 3, 'y')
Screen.display_char(0, 4, 's')
Screen.display_char(0, 5, 'd')
Screen.display_char(0, 6, 'a')
Screen.display_char(0, 7, 'r')
Screen.display_char(0, 8, 'f')
Screen.display_char(0, 9, 't')
Screen.display_char(24, 67, '#')
Screen.sleep(3)
Screen.stop_service()
