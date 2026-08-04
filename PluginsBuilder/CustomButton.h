#pragma once
#include <wx/wx.h>

class CustomButton : public wxButton
{
public:
    CustomButton(wxWindow* parent, wxWindowID id, const wxString& label,
                 const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

    void SetNormalColor(const wxColour& color);
    void SetHoverColor(const wxColour& color);
    void SetPressedColor(const wxColour& color);
    void SetTextColor(const wxColour& color);

protected:
    void OnEnter(wxMouseEvent& event);
    void OnLeave(wxMouseEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnPaint(wxPaintEvent& event);

    wxColour m_normalColor = *wxWHITE;
    wxColour m_hoverColor = *wxLIGHT_GREY;
    wxColour m_pressedColor = *wxLIGHT_GREY;
    wxColour m_textColor = *wxBLACK;
    bool m_isHovered = false;
    bool m_isPressed = false;

    void UpdateBackground();

    wxDECLARE_EVENT_TABLE();
};


