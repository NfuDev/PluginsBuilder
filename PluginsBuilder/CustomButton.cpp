#include "CustomButton.h"


wxBEGIN_EVENT_TABLE(CustomButton, wxButton)
EVT_ENTER_WINDOW(CustomButton::OnEnter)
EVT_LEAVE_WINDOW(CustomButton::OnLeave)
EVT_LEFT_DOWN(CustomButton::OnLeftDown)
EVT_LEFT_UP(CustomButton::OnLeftUp)
EVT_PAINT(CustomButton::OnPaint)
wxEND_EVENT_TABLE()

CustomButton::CustomButton(wxWindow* parent, wxWindowID id, const wxString& label,
    const wxPoint& pos, const wxSize& size)
    : wxButton(parent, id, label, pos, size)
{
    SetBackgroundColour(m_normalColor);
}

void CustomButton::SetNormalColor(const wxColour& color)
{
    m_normalColor = color;
    UpdateBackground();
}

void CustomButton::SetHoverColor(const wxColour& color)
{
    m_hoverColor = color;
}

void CustomButton::SetPressedColor(const wxColour& color)
{
    m_pressedColor = color;
}

void CustomButton::SetTextColor(const wxColour& color)
{
    m_textColor = color;
    SetForegroundColour(m_textColor);
}

void CustomButton::OnEnter(wxMouseEvent& event)
{
    m_isHovered = true;
    UpdateBackground();
    event.Skip();
}

void CustomButton::OnLeave(wxMouseEvent& event)
{
    m_isHovered = false;
    m_isPressed = false;
    UpdateBackground();
    event.Skip();
}

void CustomButton::OnLeftDown(wxMouseEvent& event)
{
    m_isPressed = true;
    UpdateBackground();
    // Force a full repaint to restore button shape and text after dialog
    Refresh();
    event.Skip();
}

void CustomButton::OnLeftUp(wxMouseEvent& event)
{
    m_isPressed = false;
    UpdateBackground();
    SetForegroundColour(m_normalColor);
    Refresh();
    event.Skip();
}

void CustomButton::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    wxColour color = m_normalColor;

    if (m_isPressed)
        color = m_pressedColor;
    else if (m_isHovered)
        color = m_hoverColor;

    dc.SetBrush(wxBrush(color));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(GetClientRect());

    dc.SetTextForeground(m_textColor);
    dc.SetFont(GetFont());
    wxString label = GetLabel();
    wxSize textSize = dc.GetTextExtent(label);
    wxPoint pos((GetSize().x - textSize.x) / 2, (GetSize().y - textSize.y) / 2);
    dc.DrawText(label, pos);

    event.Skip();
}

void CustomButton::UpdateBackground()
{
    wxColour color = m_normalColor;
    if (m_isPressed)
        color = m_pressedColor;
    else if (m_isHovered)
        color = m_hoverColor;
    SetBackgroundColour(color);
    SetForegroundColour(m_isHovered ? m_normalColor : m_textColor);
    Refresh();
}
