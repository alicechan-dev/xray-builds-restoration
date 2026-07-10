#include "MainFrame.h"

#include <wx/app.h>

class wxSDKEditorApp final : public wxApp
{
public:
    bool OnInit() override
    {
        auto* frame = new wxSDKEditorFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(wxSDKEditorApp);
