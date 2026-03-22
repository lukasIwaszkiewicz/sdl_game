#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <RmlUi_Backend.h>

// ---------------------------------------------------------------------------
// Helper: load a .ttf font (bundled with RmlUi samples or supply your own)
// ---------------------------------------------------------------------------
static void LoadFonts()
{
    // RmlUi needs at least one font to render text.
    const Rml::String font_dir = "assets/fonts/";

    // Load a primary font
    Rml::LoadFontFace(font_dir + "LatoLatin-Regular.ttf", true);
    Rml::LoadFontFace(font_dir + "LatoLatin-Bold.ttf", false);
    Rml::LoadFontFace(font_dir + "LatoLatin-Italic.ttf", false);
    Rml::LoadFontFace(font_dir + "LatoLatin-BoldItalic.ttf", false);
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
int main(int /*argc*/, char** /*argv*/)
{
    constexpr int width  = 1280;
    constexpr int height = 720;

    // 1. Initialize the backend (creates SDL window + OpenGL context)
    if (!Backend::Initialize("SDL3 + RmlUi", width, height, true))
        return -1;

    // 2. Install RmlUi interfaces
    Rml::SetSystemInterface(Backend::GetSystemInterface());
    Rml::SetRenderInterface(Backend::GetRenderInterface());

    // 3. Initialize RmlUi
    Rml::Initialise();

    // 4. Create an RmlUi context matching the window size
    Rml::Context* context = Rml::CreateContext("main", Rml::Vector2i(width, height));
    if (!context)
    {
        Rml::Shutdown();
        Backend::Shutdown();
        return -1;
    }

    // 5. Initialize the visual debugger (F8 to toggle)
    Rml::Debugger::Initialise(context);

    // 6. Load fonts
    LoadFonts();

    // 7. Load the sample document
    Rml::ElementDocument* document = context->LoadDocument("assets/hello.rml");
    if (document)
        document->Show();

    // ── Main loop ───────────────────────────────────────────────────────────
    bool running = true;
    while (running)
    {
        running = Backend::ProcessEvents(context, nullptr, true);

        context->Update();

        Backend::BeginFrame();
        context->Render();
        Backend::PresentFrame();
    }

    // ── Cleanup ─────────────────────────────────────────────────────────────
    Rml::Shutdown();
    Backend::Shutdown();

    return 0;
}
