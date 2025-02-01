// TerminalDisplay.cpp
#include <TerminalDisplay.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <ASCIIKeymap.h>

TerminalDisplay::TerminalDisplay() : mainLoopWorker(this, &TerminalDisplay::mainLoop)
{
    log("Instantiating display...\n");
}

TerminalDisplay::~TerminalDisplay()
{
    log("Cleanup terminal display...");
    cleanup();
    log("done\n");
}

void TerminalDisplay::init()
{
    if (is_inited) {
        return;
    }

    log("Initializing terminal display...\n");
    is_inited = true;
    mainLoopWorker.start();
}

void TerminalDisplay::cleanup()
{
    if (!is_inited) {
        return;
    }

    log("Cleaning up GUI display...\n");
    is_inited = false;
    mainLoopWorker.stop();
}

/**
 * @brief Converts an SFML key code along with shift and control states into a KeyCode.
 *
 * When the control flag is true and the key is one of A–Z, it maps the key to
 * ASCII 1–26 (i.e. Ctrl+A → 1, Ctrl+B → 2, ... Ctrl+Z → 26). Otherwise, it uses
 * the shift flag to determine whether to return an uppercase or lowercase letter
 * (or the shifted/unshifted version of a digit or punctuation).
 *
 * @param key     The SFML key (from sf::Event::KeyPressed).
 * @param shift   True if the Shift key is held.
 * @param control True if the Control key is held.
 * @return The corresponding KeyCode from the keymap, or NO_KEY if no mapping exists.
 */
KeyCode convert_sfml_key(sf::Keyboard::Key key, bool shift, bool control) {
    // If Control is pressed and key is a letter, map to ASCII 1-26.
    if (control) {
        if (key >= sf::Keyboard::A && key <= sf::Keyboard::Z) {
            // Map A->1, B->2, ..., Z->26.
            return static_cast<KeyCode>((key - sf::Keyboard::A) + 1);
        }
        // You can add additional control mappings here if needed.
        return NO_KEY;
    }

    // Otherwise, process as normal.
    switch(key) {
        // Letters: return uppercase if shift is pressed, lowercase otherwise.
        case sf::Keyboard::A: return shift ? ASCII_A : ASCII_a;
        case sf::Keyboard::B: return shift ? ASCII_B : ASCII_b;
        case sf::Keyboard::C: return shift ? ASCII_C : ASCII_c;
        case sf::Keyboard::D: return shift ? ASCII_D : ASCII_d;
        case sf::Keyboard::E: return shift ? ASCII_E : ASCII_e;
        case sf::Keyboard::F: return shift ? ASCII_F : ASCII_f;
        case sf::Keyboard::G: return shift ? ASCII_G : ASCII_g;
        case sf::Keyboard::H: return shift ? ASCII_H : ASCII_h;
        case sf::Keyboard::I: return shift ? ASCII_I : ASCII_i;
        case sf::Keyboard::J: return shift ? ASCII_J : ASCII_j;
        case sf::Keyboard::K: return shift ? ASCII_K : ASCII_k;
        case sf::Keyboard::L: return shift ? ASCII_L : ASCII_l;
        case sf::Keyboard::M: return shift ? ASCII_M : ASCII_m;
        case sf::Keyboard::N: return shift ? ASCII_N : ASCII_n;
        case sf::Keyboard::O: return shift ? ASCII_O : ASCII_o;
        case sf::Keyboard::P: return shift ? ASCII_P : ASCII_p;
        case sf::Keyboard::Q: return shift ? ASCII_Q : ASCII_q;
        case sf::Keyboard::R: return shift ? ASCII_R : ASCII_r;
        case sf::Keyboard::S: return shift ? ASCII_S : ASCII_s;
        case sf::Keyboard::T: return shift ? ASCII_T : ASCII_t;
        case sf::Keyboard::U: return shift ? ASCII_U : ASCII_u;
        case sf::Keyboard::V: return shift ? ASCII_V : ASCII_v;
        case sf::Keyboard::W: return shift ? ASCII_W : ASCII_w;
        case sf::Keyboard::X: return shift ? ASCII_X : ASCII_x;
        case sf::Keyboard::Y: return shift ? ASCII_Y : ASCII_y;
        case sf::Keyboard::Z: return shift ? ASCII_Z : ASCII_z;

        // Digits: if shift is pressed, return the typical symbol.
        case sf::Keyboard::Num0:
            return shift ? ASCII_RIGHT_PAREN : ASCII_0; // ')' when shifted
        case sf::Keyboard::Num1:
            return shift ? ASCII_EXCLAMATION : ASCII_1;  // '!' when shifted
        case sf::Keyboard::Num2:
            return shift ? ASCII_AT : ASCII_2;           // '@'
        case sf::Keyboard::Num3:
            return shift ? ASCII_HASH : ASCII_3;          // '#'
        case sf::Keyboard::Num4:
            return shift ? ASCII_DOLLAR : ASCII_4;        // '$'
        case sf::Keyboard::Num5:
            return shift ? ASCII_PERCENT : ASCII_5;       // '%'
        case sf::Keyboard::Num6:
            return shift ? ASCII_CARET : ASCII_6;         // '^'
        case sf::Keyboard::Num7:
            return shift ? ASCII_AMPERSAND : ASCII_7;     // '&'
        case sf::Keyboard::Num8:
            return shift ? ASCII_ASTERISK : ASCII_8;      // '*'
        case sf::Keyboard::Num9:
            return shift ? ASCII_LEFT_PAREN : ASCII_9;    // '('

        // Punctuation and other common keys:
        case sf::Keyboard::Space:
            return ASCII_SPACE;
        case sf::Keyboard::Comma:
            return shift ? ASCII_LESS_THAN : ASCII_COMMA; // '<' if shifted, ',' otherwise
        case sf::Keyboard::Period:
            return shift ? ASCII_GREATER_THAN : ASCII_PERIOD; // '>' if shifted, '.' otherwise
        case sf::Keyboard::Slash:
            return shift ? ASCII_QUESTION : ASCII_SLASH;  // '?' if shifted, '/' otherwise
        case sf::Keyboard::BackSlash:
            return ASCII_BACKSLASH; // Usually the same regardless of shift.
        case sf::Keyboard::SemiColon:
            return shift ? ASCII_COLON : ASCII_SEMICOLON; // ':' if shifted, ';' otherwise
        case sf::Keyboard::Quote:
            return shift ? ASCII_DOUBLE_QUOTE : ASCII_SINGLE_QUOTE; // double vs. single quote
        case sf::Keyboard::Tilde:
            return shift ? ASCII_TILDE : ASCII_BACKTICK; // '~' vs. '`'
        case sf::Keyboard::Equal:
            return shift ? ASCII_PLUS : ASCII_EQUAL;  // '+' if shifted, '=' otherwise
        case sf::Keyboard::Hyphen:
            return shift ? ASCII_UNDERSCORE : ASCII_MINUS; // '_' if shifted, '-' otherwise

        // For keys that we do not handle explicitly, return NO_KEY.
        default:
            return NO_KEY;
    }
}

// Define a debounce interval (e.g., 50 milliseconds).
constexpr auto debounceInterval = std::chrono::milliseconds(10);

// Global (or static) map to store the last event time for each key.
static std::unordered_map<sf::Keyboard::Key, std::chrono::steady_clock::time_point> lastKeyTime;

KeyCode processKeyEvent(const sf::Event::KeyEvent& keyEvent)
{
    // Get current time.
    const auto now = std::chrono::steady_clock::now();

    // Check if we've seen this key recently.
    auto it = lastKeyTime.find(keyEvent.code);
    if (it != lastKeyTime.end())
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second);
        if (elapsed < debounceInterval) {
            // Ignore the event as it came too soon after the last one.
            return NO_KEY;
        }
    }
    // Record the time for this key.
    lastKeyTime[keyEvent.code] = now;

    // Now convert the SFML key event into our KeyCode.
    // Assume you have a function convert_sfml_key(...) defined as before:
    const bool shift = keyEvent.shift;
    const bool control = keyEvent.control;
    return convert_sfml_key(keyEvent.code, shift, control);
}

void TerminalDisplay::mainLoop(std::atomic<bool> & running_)
{
    // Virtual resolution for the 80x25 grid.
    constexpr int columns = 80;
    constexpr int rows = 25;
    constexpr int totalChars = columns * rows; // 2000
    // Define cell size (in pixels) for each character.
    constexpr float cellWidth = 10.f;
    constexpr float cellHeight = 20.f;
    const sf::Vector2u virtualSize { static_cast<unsigned int>(columns * cellWidth),
                                       static_cast<unsigned int>(rows * cellHeight) };

    // Create an SFML window.
    sf::RenderWindow window(sf::VideoMode(virtualSize.x, virtualSize.y),
                            "Sysdarft Terminal",
                            sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);

    // Limit the framerate.
    window.setFramerateLimit(60);

    // Create a view that always shows the virtual resolution.
    sf::View view(sf::FloatRect(0.f, 0.f, static_cast<float>(virtualSize.x),
                                  static_cast<float>(virtualSize.y)));
    window.setView(view);

    // Load a monospaced font.
    sf::Font font;
    if (!font.loadFromFile("JetBrainsMono-Medium.ttf")) {
        log("Error: Could not load font file \"JetBrainsMono-Medium.ttf\".\n");
        throw TerminalDisplayError("Could not load font."); // Abort
    }

    // Pre-create SFML Text objects for each character.
    std::vector<sf::Text> charTexts;
    charTexts.reserve(totalChars);
    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < columns; ++col)
        {
            sf::Text text;
            text.setFont(font);
            text.setCharacterSize(static_cast<unsigned int>(cellHeight)); // Use cellHeight as font size.
            text.setFillColor(sf::Color::Cyan);  // Color can be adjusted for the analog horror effect.
            text.setPosition(static_cast<float>(col) * cellWidth, static_cast<float>(row) * cellHeight);
            text.setString(" "); // Initialize with a space.
            charTexts.push_back(text);
        }
    }

    // Main loop.
    while (running_)
    {
        if (!window.isOpen()) {
            break;
        }

        sf::Event event{};
        while (window.pollEvent(event))
        {
            // Handle window close event.
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            // Handle window resize event: maintain aspect ratio.
            else if (event.type == sf::Event::Resized)
            {
                float windowRatio = static_cast<float>(event.size.width) / static_cast<float>(event.size.height);
                float viewRatio = static_cast<float>(virtualSize.x) / static_cast<float>(virtualSize.y);
                float scale = 1.f;
                sf::FloatRect viewRect;
                if (windowRatio >= viewRatio) {
                    // Window is wider than virtual aspect ratio.
                    scale = static_cast<float>(event.size.height) / static_cast<float>(virtualSize.y);
                    float newWidth = static_cast<float>(event.size.width) / scale;
                    viewRect = sf::FloatRect((
                        static_cast<float>(virtualSize.x) - newWidth) / 2, 0, newWidth,
                        static_cast<float>(virtualSize.y));
                } else {
                    // Window is taller than virtual aspect ratio.
                    scale = static_cast<float>(event.size.width) / static_cast<float>(virtualSize.x);
                    float newHeight = static_cast<float>(event.size.height) / scale;
                    viewRect = sf::FloatRect(0, (
                        static_cast<float>(virtualSize.y) - newHeight) / 2,
                        static_cast<float>(virtualSize.x), newHeight);
                }
                view = sf::View(viewRect);
                window.setView(view);
            }
            // Handle key pressed events.
            else if (event.type == sf::Event::KeyPressed)
            {
                const int kc = processKeyEvent(event.key);
                if (kc != -1)
                {
                    if (input_handler_) {
                        input_handler_(kc);
                    }
                }
            }
            // Handle text input for normal characters.
            // else if (event.type == sf::Event::TextEntered)
            // {
            //     // Filter out non-printable characters except newline, etc.
            //     if (event.text.unicode < 128) {
            //         int ch = static_cast<int>(event.text.unicode);
            //         if (input_handler_) {
            //             input_handler_(ch);
            //         }
            //     }
            // }
        }

        // Clear the window.
        window.clear(sf::Color::Black);

        // If a video memory reader has been registered, update the text.
        if (buffer_reader_)
        {
            std::array<char, totalChars> buffer = buffer_reader_();
            for (int i = 0; i < totalChars; ++i) {
                // Convert char to a string for sf::Text.
                char c = buffer[i];
                char s[2] = { c, '\0' };
                charTexts[i].setString(s);
            }
        }

        // Draw all characters.
        for (const auto& text : charTexts) {
            window.draw(text);
        }

        window.display();

        // Sleep a little bit to yield CPU.
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    log("Window closed.\n");
    window.close();

    log("Display thread quit.\n");
}
