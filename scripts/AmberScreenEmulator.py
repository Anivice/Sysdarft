#!/usr/bin/env python

import pygame
import pygame.freetype
from PIL import Image, ImageFilter
import threading
import time

class AmberScreenEmulator:
    def __init__(self,
                 caption='Amber Phosphor Screen Emulator',
                 font='Courier',
                 screen_width=1366,
                 screen_height=768,
                 cols=50,
                 rows=25):
        # Initialize pygame
        pygame.init()

        self.running_verification_flag = False

        # Screen settings
        self.screen_width = screen_width
        self.screen_height = screen_height
        self.cols = cols
        self.rows = rows
        self.char_width = screen_width // cols
        self.char_height = screen_height // rows

        # Font settings
        self.font_size = min(self.char_width, self.char_height)
        self.font = pygame.freetype.SysFont(font, self.font_size)
        self.amber_color = (255, 191, 0)
        self.bg_color = (16, 2, 0)

        # Decay settings
        self.decay_buffer = [['' for _ in range(cols)] for _ in range(rows)]
        self.decay_timer = [[0 for _ in range(cols)] for _ in range(rows)]
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

        # Start text input for pygame
        pygame.key.start_text_input()

        # Create clock for FPS control
        self.clock = pygame.time.Clock()
        self.FPS = 60

    def add_glow(self, image_surface, blur_radius=8):
        pil_string_image = pygame.image.tostring(image_surface, "RGB")
        pil_image = Image.frombytes("RGB", image_surface.get_size(), pil_string_image)
        pil_image = pil_image.filter(ImageFilter.GaussianBlur(blur_radius))
        return pygame.image.fromstring(pil_image.tobytes(), pil_image.size, "RGB")

    def display_char(self, row, col, char):
        if self.decay_buffer[row][col] != char:
            self.decay_buffer[row][col] = char
            self.decay_timer[row][col] = self.max_decay_time
            if (row, col) not in self.active_decay_positions:
                self.active_decay_positions.append((row, col))

    def update_decay(self):
        for (row, col) in self.active_decay_positions[:]:
            if self.decay_timer[row][col] > 0:
                self.decay_timer[row][col] -= 1

    def render_pixelated_text(self, text, color, scale_factor=1.5):
        small_surface = pygame.Surface((self.char_width, self.char_height))
        small_surface.fill(self.bg_color)
        self.font.render_to(small_surface, (0, 0), text, color)

        pixelated_surface = pygame.transform.scale(
            small_surface,
            (int(self.char_width // scale_factor), int(self.char_height // scale_factor))
        )

        final_surface = pygame.transform.scale(pixelated_surface, (self.char_width, self.char_height))
        return final_surface

    def service_loop(self):
        self.running_verification_flag = True

        try:
            while self.running_event.is_set():
                self.screen.fill(self.bg_color)

                # Display the grid of text with a decay effect
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

                # Update decay and apply glow effect
                self.update_decay()
                self.glow_surface = self.add_glow(self.screen)
                self.screen.blit(self.glow_surface, (0, 0), special_flags=pygame.BLEND_ADD)

                # Handle events
                self.handle_events()

                # Update display and control frame rate
                pygame.display.flip()
                self.clock.tick(self.FPS)

            # service finished
            self.cleanup()

            self.running_verification_flag = False

        except Exception as err:
            # Log the error and stop the event loop if an exception occurs
            print(f"An error occurred during the service loop: {err}")
            self.running_event.clear()

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running_event.clear()
            elif event.type == pygame.TEXTINPUT:
                character = event.text[0]
                if ord(character) in range(0, 256):
                    self.input_stream += ["Input", [character, ""]]
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
            service_thread = threading.Thread(target=self.service_loop)
            service_thread.start()
            return 0

        except Exception as err:
            print(f'Error when starting service ({err})!')
            return -1

    def stop_service(self):
        try:
            self.running_event.clear()
            return 0
        except Exception as err:
            print(f'Error when starting service ({err})!')
            return -1

    def query_input_stream(self):
        return self.input_stream

    def input_stream_pop_front(self):
        if self.input_stream:
            self.input_stream.pop(0)

    def join_service_loop(self):
        while self.running_event.is_set() or self.running_verification_flag:
            time.sleep(0.1)

    def sleep(self, seconds):
        time.sleep(seconds)