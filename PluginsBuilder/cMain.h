#pragma once
#include "wx/wx.h"

#include <wx/listbox.h>
#include <wx/filedlg.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/xml/xml.h>
#include <wx/filename.h>
#include <wx/process.h>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>
#include <wx/sstream.h>
#include <wx/statbmp.h>
#include <vector>


#include <wx/dir.h>
#include <wx/textfile.h>

#include <wx/listctrl.h> 
#include <wx/filepicker.h>

struct Entry
{
    wxString Name;
    wxString Path;
    wxString version;

	wxDateTime LastTimeBuilt;

    const bool operator==(const Entry& other) const
    {
        return Name == other.Name && Path == other.Path && version == other.version;
    }

    wxStaticText* LastBuiltText = nullptr;
    
    void UpdateLastBuiltText(class cMain* parent);

};

struct TodoEntry
{
    wxString File;
    long Line = -1;
    wxString Context;
    wxString Type;
};

class EntryDialog : public wxDialog
{
public:
    EntryDialog(wxWindow* parent);

    wxString GetEntryName() const { return m_entryName; }
    wxString GetEntryPath() const { return m_entryPath; }
    wxString GetEntryVersion() const { return m_entryVersion; }

    void SetEntryName   (wxString inValue)     {  m_entryName = inValue; }
    void SetEntryPath   (wxString inValue)     {  m_entryPath = inValue; }
    void SetEntryVersion(wxString inValue)     {  m_entryVersion = inValue; }

private:
 
    wxString m_entryName;
    wxString m_entryPath;
    wxString m_entryVersion;
};

class ToDO : public wxDialog
{
public:

    ToDO(wxWindow* parent, Entry& entry);

    wxPanel* TodoPanel = nullptr;
    wxDirPickerCtrl* TodoDirPicker = nullptr;
    wxButton* TodoScanButton = nullptr;
    wxListCtrl* TodoList = nullptr;
    wxButton* ShowTodoButton = nullptr;

    void ScanTodosInDir(const wxString& dirPath, std::vector<TodoEntry>& outEntries);
    void FindTodosInFile(const wxString& filePath, std::vector<TodoEntry>& outEntries);
   // void OnShowTodoButton(wxCommandEvent& event);
    void OnTodoActivated(wxListEvent& event);

    inline wxString ReplaceLastWithSource(const wxString& path)
    {
        wxString normalized = path;
        normalized.Replace("\\", "/");

        int pos = normalized.Find('/', true);

        if (pos == wxNOT_FOUND)
            return "Source";

        wxString base = normalized.Left(pos);

        return base + "/Source";
    }
};

class EngineVersionDialog : public wxDialog
{
public:
    EngineVersionDialog(wxWindow* parent);
};

class cMain : public wxFrame
{
 /*
public:
    wxScrolledWindow* LeftPanel;
    wxTextCtrl* ConsoleOutput;
    wxTextCtrl* BuildCommands;
    wxBoxSizer* EntrySizer;
    wxBoxSizer* EngineTargets;
	int CurrentBuildIndex = -1;
    std::vector<Entry> Entries;
    std::vector<wxString> EngineVersions;   
    wxString XmlPath;
    wxString ResultsPath;

    wxProcess* m_process;

    void LoadEntries();
    void SaveEntries();
    void LoadEngines();                     
    void SaveEngines();                     
    void RefreshEntries();
	void RefreshEngineTargets();

	bool CheckPathValidity(const wxString& path);


    void OnAddEntry(wxCommandEvent&);
    void OnNewEngineVersion(wxCommandEvent&); 
    void OnCancelBuild(wxCommandEvent&);      

    void RunBuild(const wxString& pluginPath, const wxString& versionName, const wxString& version, const wxString& enginePath);

    void OnProcessTerminated(wxProcessEvent& event);
    void OnIdle(wxIdleEvent& event);

    wxString GetTimeAgoString(const wxDateTime& lastBuilt);

    wxTimer m_updateTimer;
	void OnUpdateTimer(wxTimerEvent& event);*/

    ///=========================================================================
public:
    cMain();
    ~cMain();


public:
    wxScrolledWindow* LeftPanel;
    wxTextCtrl* ConsoleOutput;
    wxTextCtrl* BuildCommands;
    wxBoxSizer* EntrySizer;
    wxBoxSizer* EngineTargets;
    int CurrentBuildIndex = -1;
    std::vector<Entry> Entries;
    std::vector<wxString> EngineVersions;
    wxString XmlPath;
    wxString ResultsPath;


    wxProcess* m_process = nullptr;


    // TODO panel controls
    wxPanel* TodoPanel = nullptr;
    wxDirPickerCtrl* TodoDirPicker = nullptr;
    wxButton* TodoScanButton = nullptr;
    wxListCtrl* TodoList = nullptr;
    wxButton* ShowTodoButton = nullptr; // toggles view


    void LoadEntries();
    void SaveEntries();
    void LoadEngines();
    void SaveEngines();
    void RefreshEntries();
    void RefreshEngineTargets();


    bool CheckPathValidity(const wxString& path);




    void OnAddEntry(wxCommandEvent&);
    void OnToDOPressed(wxCommandEvent&, Entry& entry);
    void OnNewEngineVersion(wxCommandEvent&);
    void OnCancelBuild(wxCommandEvent&);


    void RunBuild(const wxString& pluginPath, const wxString& versionName, const wxString& version, const wxString& enginePath);


    void OnProcessTerminated(wxProcessEvent& event);
    void OnIdle(wxIdleEvent& event);


    wxString GetTimeAgoString(const wxDateTime& lastBuilt);


    wxTimer m_updateTimer;
    void OnUpdateTimer(wxTimerEvent& event);


    // TODO scanning helpers
    void ScanTodosInDir(const wxString& dirPath, std::vector<TodoEntry>& outEntries);
    void FindTodosInFile(const wxString& filePath, std::vector<TodoEntry>& outEntries);
    void OnShowTodoButton(wxCommandEvent& event);
    void OnTodoActivated(wxListEvent& event);
};