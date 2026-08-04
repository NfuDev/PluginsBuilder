#include "cMain.h"
#include <wx/splitter.h>
#include <wx/dir.h>
#include <wx/image.h>
#include <wx/statline.h>
#include "resource.h"
#include <wx/datetime.h>
#include "CustomButton.h"


#define ADD_LINE_SPLIT(sizer,parent)\
sizer->Add(new wxStaticLine(parent, wxID_ANY),\
0, wxEXPAND | wxTOP | wxBOTTOM, 5)\


#define ALLOW_DOUBLICATE_ENTRIES false

EntryDialog::EntryDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Add Plugin Entry", wxDefaultPosition, wxSize(400, 200))
{
 
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxButton* browseBtn = new wxButton(this, wxID_ANY, "Browse for .uplugin");
    wxTextCtrl* pathInput = new wxTextCtrl(this, wxID_ANY);
    wxTextCtrl* versionInput = new wxTextCtrl(this, wxID_ANY);
    wxTextCtrl* NameInput = new wxTextCtrl(this, wxID_ANY);

    pathInput->SetValidator(wxTextValidator(wxFILTER_EMPTY, &m_entryPath));
    NameInput->SetValidator(wxTextValidator(wxFILTER_EMPTY, &m_entryName));
    versionInput->SetValidator(wxTextValidator(wxFILTER_EMPTY, &m_entryVersion));

    sizer->Add(browseBtn, 0, wxEXPAND | wxALL, 5);
    sizer->Add(pathInput, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxStaticText(this, wxID_ANY, "Plugin Name:"), 0, wxALL, 5);
    sizer->Add(NameInput, 0, wxEXPAND | wxALL, 5);
    sizer->Add(new wxStaticText(this, wxID_ANY, "Version Name:"), 0, wxALL, 5);
    sizer->Add(versionInput, 0, wxEXPAND | wxALL, 5);

    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();
    buttons->AddButton(new wxButton(this, wxID_OK));
    buttons->AddButton(new wxButton(this, wxID_CANCEL));
    buttons->Realize();

    sizer->Add(buttons, 0, wxALIGN_CENTER | wxALL, 5);

    SetSizerAndFit(sizer);

    browseBtn->Bind(wxEVT_BUTTON, [this, pathInput](wxCommandEvent&)
        {
            wxFileDialog openFileDialog(this, "Select .uplugin", "", "",
                "UPlugin files (*.uplugin)|*.uplugin", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
            if (openFileDialog.ShowModal() == wxID_OK)
            {
                pathInput->SetValue(openFileDialog.GetPath());
            }
        });
}

EngineVersionDialog::EngineVersionDialog(wxWindow* parent) : wxDialog(parent, wxID_ANY, "Supported Engine Versions", wxDefaultPosition, wxSize(900, 500))
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);


    sizer->Add(new wxStaticText(this, wxID_ANY, "Plugin Name:"), 1, wxALL, 5);
    sizer->Add(new wxStaticText(this, wxID_ANY, "Version Name:"), 1, wxALL, 5);


    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();
    buttons->AddButton(new wxButton(this, wxID_CLOSE));
    buttons->Realize();

    sizer->Add(buttons, 1, wxALIGN_CENTER | wxALL, 5);

    SetSizerAndFit(sizer,false);
   // CenterOnParent();
}


// ---------------------- MAIN ----------------------

cMain::cMain()
    : wxFrame(nullptr, wxID_ANY, "Arrows Plugins Builder", wxDefaultPosition, wxSize(1280,720)), m_process(nullptr)
{
    wxImage::AddHandler(new wxPNGHandler);

    //widget tick--------------------------------
    m_updateTimer.SetOwner(this);
    m_updateTimer.Start(60000);
    Bind(wxEVT_TIMER, &cMain::OnUpdateTimer, this);
	//----------------------

    wxMenu* fileMenu = new wxMenu;
    fileMenu->Append(wxID_NEW, "&New Engine Version", "Create new target engine version to build to");
    Bind(wxEVT_MENU, &cMain::OnNewEngineVersion, this, wxID_NEW);

   
    wxIcon appIcon;
    appIcon.LoadFile("PluginsBuilder.ico", wxBITMAP_TYPE_RESOURCE);
    if (appIcon.IsOk())
        SetIcon(appIcon);
    else
       // wxLogMessage("Failed to load icon.");

    SetIcon(wxICON(IDI_ICON1));

	EngineTargets = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* EngineTargetsSizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* Targetslable = new wxStaticText(this, wxID_ANY, "Engine Targets:");
    Targetslable->SetForegroundColour(*wxWHITE);
	EngineTargetsSizer->Add(Targetslable, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	EngineTargetsSizer->Add(EngineTargets, 1, wxEXPAND | wxALL, 5);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    SetMenuBar(menuBar);

  ///@TODO : we need custom dailogue here so users when i realease this to the public can update where the xml is saved
    wxString appDataDir = wxStandardPaths::Get().GetUserLocalDataDir();
    wxString arrowsDir = wxFileName(appDataDir).GetPath() + "\\ArrowsInteractiveStudio";
    if (!wxDirExists(arrowsDir)) {
        wxMkdir(arrowsDir);
    }

    XmlPath = arrowsDir + "\\PluginsBuilder.ADU";

    ResultsPath = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();

    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer* LeftMainSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* addButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    CustomButton* addButton = new CustomButton(this, wxID_ANY, "+ Add Plugin");

	addButton->SetNormalColor(wxColor(0,162,232));
	addButton->SetTextColor(*wxWHITE);
	//addButton->SetForegroundColour(*wxWHITE);

    addButtonSizer->Add(addButton, 1, wxEXPAND | wxALL, 5);

    LeftPanel = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL);
    LeftPanel->SetScrollRate(10, 10);

    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    EntrySizer = new wxBoxSizer(wxVERTICAL);
    leftSizer->Add(EntrySizer, 1, wxEXPAND | wxALL, 5);
    LeftPanel->SetSizer(leftSizer);

    LeftMainSizer->Add(addButtonSizer, 0, wxEXPAND | wxALL, 0);
    LeftMainSizer->Add(LeftPanel, 1, wxEXPAND | wxALL, 0);

    mainSizer->Add(LeftMainSizer, 1, wxEXPAND | wxALL, 0);

    ConsoleOutput = new wxTextCtrl(this, wxID_ANY, "",
        wxDefaultPosition, wxDefaultSize,
        wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);

    ConsoleOutput->SetBackgroundColour(*wxBLACK);
    ConsoleOutput->SetForegroundColour(*wxWHITE);

    BuildCommands = new wxTextCtrl(this, wxID_ANY);

    BuildCommands->SetValue("-nocompileuat");

    wxBoxSizer* CommandsSizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* lable = new wxStaticText(this,wxID_ANY, "Build Commands:");
    lable->SetForegroundColour(*wxWHITE);
    CommandsSizer->Add(lable,0, wxALIGN_CENTER);
    CommandsSizer->Add(BuildCommands, 1, wxEXPAND | wxALL, 5);

    wxBoxSizer* ConsoleSizer = new wxBoxSizer(wxVERTICAL);

   // mainSizer->Add(LeftPanel, 1, wxEXPAND);
    mainSizer->Add(ConsoleSizer, 2, wxEXPAND | wxALL, 0);

    ConsoleSizer->Add(CommandsSizer, 0, wxEXPAND | wxALL, 5);
    ConsoleSizer->Add(EngineTargetsSizer, 0, wxEXPAND | wxALL, 5);
    ConsoleSizer->Add(ConsoleOutput, 1, wxEXPAND | wxALL,5);

    wxButton* cancelButton = new wxButton(this, wxID_ANY, "Cancel Build");
    ConsoleSizer->Add(cancelButton, 0, wxALIGN_RIGHT | wxALL, 5);
    cancelButton->Bind(wxEVT_BUTTON, &cMain::OnCancelBuild, this);

    SetSizer(mainSizer);

    //// ========== TODO PANEL (hidden by default) ==========
    //TodoPanel = new wxPanel(this, wxID_ANY);
    ////TodoPanel->Hide();


    //wxBoxSizer* todoSizer = new wxBoxSizer(wxVERTICAL);
    //TodoDirPicker = new wxDirPickerCtrl(TodoPanel, wxID_ANY, "", "Select source directory");
    //TodoScanButton = new wxButton(TodoPanel, wxID_ANY, "Scan TODOs");
    //TodoList = new wxListCtrl(TodoPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
    //    wxLC_REPORT | wxLC_SINGLE_SEL);


    //TodoList->InsertColumn(0, "File", wxLIST_FORMAT_LEFT, 400);
    //TodoList->InsertColumn(1, "Line", wxLIST_FORMAT_LEFT, 60);
    //TodoList->InsertColumn(2, "Context", wxLIST_FORMAT_LEFT, 700);


    //todoSizer->Add(TodoDirPicker, 0, wxEXPAND | wxALL, 5);
    //todoSizer->Add(TodoScanButton, 0, wxEXPAND | wxALL, 5);
    //todoSizer->Add(TodoList, 1, wxEXPAND | wxALL, 5);


    //TodoPanel->SetSizer(todoSizer);
    //mainSizer->Add(TodoPanel, 2, wxEXPAND | wxALL, 0);
    //// ====================================================

    SetBackgroundColour(wxColour(45, 45, 48));
    SetForegroundColour(*wxWHITE);
    LeftPanel->SetBackgroundColour(wxColour(37, 37, 38));
    LeftPanel->SetForegroundColour(*wxWHITE);

    addButton->Bind(wxEVT_BUTTON, &cMain::OnAddEntry, this);

    // Bind TODO scan
    //TodoScanButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
    //    {
    //        wxString dir = TodoDirPicker->GetPath();
    //        if (dir.IsEmpty())
    //        {
    //            wxMessageBox("Please select a source directory first.");
    //            return;
    //        }


    //        // Busy cursor
    //        wxBeginBusyCursor();
    //        std::vector<TodoEntry> todos;
    //        ScanTodosInDir(dir, todos);
    //        wxEndBusyCursor();


    //        TodoList->DeleteAllItems();
    //        for (size_t i = 0; i < todos.size(); ++i)
    //        {
    //            long idx = TodoList->InsertItem((long)i, todos[i].File);
    //            TodoList->SetItem(idx, 1, wxString::Format("%ld", todos[i].Line));
    //            TodoList->SetItem(idx, 2, todos[i].Context);
    //        }


    //        wxMessageBox(wxString::Format("Found %zu TODOs / FIXMEs", todos.size()));
    //    });


    // double click to open file
    //TodoList->Bind(wxEVT_LIST_ITEM_ACTIVATED, &cMain::OnTodoActivated, this);

    LoadEntries();
    LoadEngines();
    RefreshEntries();

    if(EngineVersions.empty())
    {
		EngineTargets->Add(new wxStaticText(this, wxID_ANY, "No Engine Versions Added, Go to File->New Engine Version"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    }
}

cMain::~cMain() {}


void cMain::LoadEntries()
{
    Entries.clear();
    if (!wxFileExists(XmlPath)) return;

    wxXmlDocument doc;
    if (!doc.Load(XmlPath)) return;

    wxXmlNode* root = doc.GetRoot();
    if (!root) return;

    // Load Entries
    wxXmlNode* entriesNode = root->GetChildren();
    while (entriesNode)
    {
        if (entriesNode->GetName() == "Entry")
        {
            Entry e;
            e.Name = entriesNode->GetAttribute("Name");
            e.Path = entriesNode->GetAttribute("Path");
            e.version = entriesNode->GetAttribute("Version");

            wxString lastBuiltStr = entriesNode->GetAttribute("date");
            if (lastBuiltStr != "Never built")
            {
                wxDateTime dt;
                if (dt.ParseISOCombined(lastBuiltStr))
                {
                    e.LastTimeBuilt = dt;
                }
			}
  
            Entries.push_back(e);
        }
        entriesNode = entriesNode->GetNext();
    }
}

void cMain::SaveEntries()
{
    wxXmlDocument doc;
    wxXmlNode* root = new wxXmlNode(wxXML_ELEMENT_NODE, "PluginsBuilder");
    doc.SetRoot(root);

    for (const auto& entry : Entries)
    {
        wxXmlNode* entryNode = new wxXmlNode(wxXML_ELEMENT_NODE, "Entry");
       
        entryNode->AddAttribute("Path", entry.Path);
        entryNode->AddAttribute("Name", entry.Name);
        entryNode->AddAttribute("Version", entry.version);
        
        wxString formatted = entry.LastTimeBuilt.IsValid() ? entry.LastTimeBuilt.FormatISOCombined('T') : "not Built";

        entryNode->AddAttribute("date", formatted);

        root->AddChild(entryNode);
       
    }

#if !ALLOW_DOUBLICATE_ENTRIES
    wxString LastAdded;
#endif

    for (const auto& eng : EngineVersions)
    {
#if !ALLOW_DOUBLICATE_ENTRIES
        if (LastAdded == eng) continue;
        LastAdded = eng;
#endif

        wxXmlNode* engNode = new wxXmlNode(wxXML_ELEMENT_NODE, "Engine");
        root->AddChild(engNode);
        engNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", eng));
    }

    doc.Save(XmlPath);
}

void cMain::RefreshEntries()
{
    EntrySizer->Clear(true);

    for (auto& e : Entries)
    {
        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        wxStaticBoxSizer* entrySizer = new wxStaticBoxSizer(wxVERTICAL, LeftPanel, "");

        wxFileName fn(e.Path);
        wxString iconPath = fn.GetPath() + "\\Resources\\Icon128.png";
        wxBitmap bmp;
        if (wxFileExists(iconPath))
        {
            bmp.LoadFile(iconPath, wxBITMAP_TYPE_PNG);

            if (bmp.IsOk())
            {
                wxImage img = bmp.ConvertToImage();
                img.Rescale(64, 64, wxIMAGE_QUALITY_HIGH);
                bmp = wxBitmap(img);
            }
        }
           
        wxStaticBox* staticBox = entrySizer->GetStaticBox();
       
        wxStaticBitmap* bmpCtrl = new wxStaticBitmap(staticBox, wxID_ANY, bmp,wxDefaultPosition,wxSize(64,64));
        wxButton* btn = new wxButton(staticBox, wxID_ANY, "Build");
        wxButton* Remove_btn = new wxButton(staticBox, wxID_ANY, "Remove");
        wxButton* ToDo_btn = new wxButton(staticBox, wxID_ANY, "To Do");
        wxButton* Edit_btn = new wxButton(staticBox, wxID_ANY, "Edit");

		btn->SetBackgroundColour(*wxGREEN);
		Remove_btn->SetBackgroundColour(*wxRED);


        wxBoxSizer* rowElements = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer* ButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer* VersionAndBuildTimeSizer = new wxBoxSizer(wxHORIZONTAL);

        wxString Name = wxString("Plugin name : ") + e.Name;
        wxString Version = wxString("Version : ") + e.version;

        wxString BuildTimeString = GetTimeAgoString(e.LastTimeBuilt);

        wxStaticText* PluginName = new wxStaticText(staticBox, wxID_ANY, Name, wxDefaultPosition);
        wxStaticText* PluginVersion = new wxStaticText(staticBox, wxID_ANY, Version, wxDefaultPosition);

        wxStaticText* buildTime = new wxStaticText(staticBox, wxID_ANY, BuildTimeString, wxDefaultPosition);
		e.LastBuiltText = buildTime;

        buildTime->SetForegroundColour(*wxGREEN);

        rowElements->Add(PluginName, 1, wxEXPAND);

       // rowElements->Add(PluginVersion, 0, wxEXPAND);
        //rowElements->Add(buildTime, 0, wxEXPAND);
		VersionAndBuildTimeSizer->Add(PluginVersion, 0, wxALIGN_CENTER_VERTICAL);
        VersionAndBuildTimeSizer->AddStretchSpacer(1);
		VersionAndBuildTimeSizer->Add(buildTime, 0,  wxALIGN_CENTER_VERTICAL);

        rowElements->Add(VersionAndBuildTimeSizer, 1, wxEXPAND);
        ButtonsSizer->Add(btn, 1, wxEXPAND);
        ButtonsSizer->Add(Remove_btn, 1, wxEXPAND);
        ButtonsSizer->Add(ToDo_btn, 1, wxEXPAND);
        ButtonsSizer->Add(Edit_btn, 1, wxEXPAND);
        rowElements->Add(ButtonsSizer, 0, wxEXPAND);

        row->Add(bmpCtrl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        row->Add(rowElements, 1, wxEXPAND);

        entrySizer->Add(row, 1, wxEXPAND | wxALL, 5);

        EntrySizer->Add(entrySizer, 0, wxEXPAND | wxALL, 5);

        btn->Bind(wxEVT_BUTTON, [this, &e](wxCommandEvent&)
            {
                if (EngineVersions.empty())
                {
                    wxMessageBox("No engine versions saved. Please add one first.");
                    return;
                }
           
                if (CurrentBuildIndex == wxNOT_FOUND)
                {
                    wxMessageBox("Please selection an engine version");
                    return;
                }

             /*   wxArrayString engineChoices;
                for (const auto& version : EngineVersions)
                {
                    engineChoices.Add(version);
                }*/

                wxString enginePath = EngineVersions[CurrentBuildIndex];
                RunBuild(e.Path, e.Name, e.version, enginePath);
				e.LastTimeBuilt = wxDateTime::Now();
                SaveEntries();
               /* wxSingleChoiceDialog dlg(this,
                    "Select Unreal Engine version for this build",
                    "Choose Engine Version",
                    engineChoices);

                if (dlg.ShowModal() == wxID_OK)
                {
                    wxString enginePath = dlg.GetStringSelection();
                    RunBuild(e.Path, e.Name,e.version, enginePath);
                }*/
            });

        Remove_btn->Bind(wxEVT_BUTTON, [this, e](wxCommandEvent&)
            {
                auto new_end = std::remove(Entries.begin(), Entries.end(), e);
                Entries.erase(new_end);
                SaveEntries();
                RefreshEntries();
            });

        ToDo_btn->Bind(wxEVT_BUTTON, [this, &e](wxCommandEvent&event)
            {
                OnToDOPressed(event,e);
            });

        Edit_btn->Bind(wxEVT_BUTTON, [this, &e](wxCommandEvent& event)
            {
                EntryDialog dlg(this);
                dlg.SetEntryName(e.Name);
                dlg.SetEntryPath(e.Path);
                dlg.SetEntryVersion(e.version);

                if (dlg.ShowModal() == wxID_OK)
                {
                    e.Path = dlg.GetEntryPath();
                    e.Name = dlg.GetEntryName();
                    e.version = dlg.GetEntryVersion();

                    if (e.Path.IsEmpty() || e.Name.IsEmpty())
                    {
                        wxMessageBox("Both path and name are required.", "Error", wxICON_ERROR);
                        return;
                    }

                    SaveEntries();
                    RefreshEntries();
                }
            });

    }

    LeftPanel->Layout();
}

void cMain::RefreshEngineTargets()
{
    EngineTargets->Clear(true);

    for (wxString& eng : EngineVersions)
    {
        wxString versionName = "Not Valid";
        char sub = '\\';
        int index = eng.Find(sub, true);
        if (index != wxNOT_FOUND)
        {
            versionName = eng.Right(eng.Length() - index - 1);
        }
		wxColor colour = *wxWHITE;
        if(!CheckPathValidity(eng))
        {
            versionName += " (Invalid Path)";
            colour = *wxRED;
		}
        wxCheckBox* newCheckBox = new wxCheckBox(this, wxID_ANY, versionName);
        newCheckBox->SetToolTip(eng);
        newCheckBox->SetForegroundColour(colour);
       
        newCheckBox->Bind(wxEVT_CHECKBOX, [this, newCheckBox](wxCommandEvent& event)
            {
                if (event.IsChecked())
                {
                    int index = -1;
                    for (wxSizerItem* item : EngineTargets->GetChildren())
                    {
                        index++;
                        wxCheckBox* cb = dynamic_cast<wxCheckBox*>(item->GetWindow());
                        if (cb && cb != newCheckBox)
                        {
                            cb->SetValue(false);
                        }
                        else if (cb == newCheckBox)
                        {
                            CurrentBuildIndex = index;
                        }
                    }

                   // wxMessageBox(wxString::Format("Current build target set to: %s", EngineVersions[CurrentBuildIndex]));
                }
            });

        EngineTargets->Add(newCheckBox, 0, wxEXPAND | wxALL, 5);
    }
}

bool cMain::CheckPathValidity(const wxString& path)
{
    wxDir dir(path);
    if (!dir.IsOpened())
        return false;

    wxString subdir;
    bool found = dir.GetFirst(&subdir, wxEmptyString, wxDIR_DIRS);
    while (found)
    {
        if (subdir == "Engine")
            return true;
        found = dir.GetNext(&subdir);
    }
    return false;
}

void cMain::LoadEngines()
{
    EngineVersions.clear();
    if (!wxFileExists(XmlPath)) return;

    wxXmlDocument doc;
    if (!doc.Load(XmlPath)) return;

    wxXmlNode* root = doc.GetRoot();
    if (!root) return;

    wxXmlNode* node = root->GetChildren();
    while (node)
    {
        if (node->GetName() == "Engine")
        {
#if !ALLOW_DOUBLICATE_ENTRIES
            if(!EngineVersions.empty())
            {
                wxString LastAdded = EngineVersions.back();
                if(LastAdded == node->GetNodeContent())
                {
                    node = node->GetNext();
                    continue;
			    }
			}
#endif

            EngineVersions.push_back(node->GetNodeContent());

        }
        node = node->GetNext();
    }

	RefreshEngineTargets();
}

void cMain::SaveEngines()
{
    SaveEntries();
}

void cMain::OnAddEntry(wxCommandEvent&)
{
    EntryDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        Entry e;
        e.Path = dlg.GetEntryPath();
        e.Name = dlg.GetEntryName();
        e.version = dlg.GetEntryVersion();

        // basic validation
        if (e.Path.IsEmpty() || e.Name.IsEmpty())
        {
            wxMessageBox("Both path and name are required.", "Error", wxICON_ERROR);
            return;
        }

        Entries.push_back(e);
        SaveEntries();
        RefreshEntries();
    }
}

void cMain::OnToDOPressed(wxCommandEvent&, Entry& entry)
{
    ToDO dlg(this,entry);
    if (dlg.ShowModal() == wxID_OK)
    {
 
    }
}

void cMain::OnNewEngineVersion(wxCommandEvent&)
{
    wxDirDialog dlg(this, "Select Unreal Engine root directory", "",
        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK)
    {

#if !ALLOW_DOUBLICATE_ENTRIES
        wxString LastAddedPath = !EngineVersions.empty() ? EngineVersions.back() : wxString();

        if(LastAddedPath == dlg.GetPath())
        {
            wxMessageBox("This engine version is already added.");
            return;
		}
#endif
        if (!CheckPathValidity(dlg.GetPath()))
        {
            wxMessageBox("The selected path does not appear to be a valid Unreal Engine root directory.", "Error", wxICON_ERROR);
            return;
        }

        EngineVersions.push_back(dlg.GetPath());
        SaveEngines();
        RefreshEngineTargets();

		Layout();
    }
}


void cMain::RunBuild(const wxString& pluginPath, const wxString& versionName, const wxString& version, const wxString& enginePath)
{
    wxString EngineVersion = enginePath.AfterLast('\\');
    wxString VersionNumber = EngineVersion.AfterFirst('_');

    EngineVersion.Replace(".", "_");

    wxTextFile pluginFile;
    if (pluginFile.Open(pluginPath))
    {
        for (size_t i = 0; i < pluginFile.GetLineCount(); i++)
        {
            if (pluginFile[i].Contains("\"EngineVersion\""))
            {
                pluginFile[i] = wxString::Format("\t\"EngineVersion\": \"%s\",", VersionNumber);
                break;
            }
        }

        pluginFile.Write();
        pluginFile.Close();
    }

    wxString command = wxString::Format(
        "\"%s\\Engine\\Build\\BatchFiles\\RunUAT.bat\" "
        "BuildPlugin -Plugin=\"%s\" -Package=\"%s\\Arrows Products\\%s_%s_%s\" -Rocket -VS2019 %s",
        enginePath, pluginPath, ResultsPath, versionName, version, EngineVersion, BuildCommands->GetValue());

    ConsoleOutput->Clear();

    m_process = new wxProcess(this);
    m_process->Redirect();

    long pid = wxExecute(command, wxEXEC_ASYNC, m_process);
    if (pid == 0)
    {
        ConsoleOutput->AppendText("Failed to start process.\n");
        delete m_process;
        m_process = nullptr;
        return;
    }

    Bind(wxEVT_IDLE, &cMain::OnIdle, this);
    Bind(wxEVT_END_PROCESS, &cMain::OnProcessTerminated, this);
}

void cMain::OnIdle(wxIdleEvent& event)
{
    if (!m_process) return;

    wxInputStream* out = m_process->GetInputStream();
    wxInputStream* err = m_process->GetErrorStream();

    char buffer[1024];

    auto AppendColoredText = [this](const wxString& text, const wxColour& color)
    {
        long start = ConsoleOutput->GetLastPosition();
        ConsoleOutput->AppendText(text);
        long end = ConsoleOutput->GetLastPosition();
        ConsoleOutput->SetStyle(start, end, wxTextAttr(color));
    };

    if (out && out->CanRead())
    {
        while (out->CanRead())
        {
            out->Read(buffer, sizeof(buffer) - 1);
            buffer[out->LastRead()] = '\0';
            wxString str(buffer);

            // Color coding
            if (str.Lower().Find("-warningsaserrors") != wxNOT_FOUND)
                AppendColoredText(str, *wxWHITE);
            else if (str.Lower().Find("error") != wxNOT_FOUND)
                AppendColoredText(str, *wxRED);
            else if (str.Lower().Find("warning") != wxNOT_FOUND)
                AppendColoredText(str, *wxYELLOW);
            else
                AppendColoredText(str, *wxWHITE);
        }
    }

    if (err && err->CanRead())
    {
        while (err->CanRead())
        {
            err->Read(buffer, sizeof(buffer) - 1);
            buffer[err->LastRead()] = '\0';
            wxString str(buffer);

            // Color coding
            if(str.Lower().Find("-warningsaserrors") != wxNOT_FOUND)
                AppendColoredText(str, *wxWHITE);
            else if (str.Lower().Find("error") != wxNOT_FOUND)
                AppendColoredText(str, *wxRED);
            else if (str.Lower().Find("warning") != wxNOT_FOUND)
                AppendColoredText(str, *wxYELLOW);
            else
                AppendColoredText(str, *wxWHITE);
        }
    }

    event.RequestMore();
}

void cMain::OnProcessTerminated(wxProcessEvent& event)
{
    ConsoleOutput->AppendText("\nProcess finished.\n");

    delete m_process;
    m_process = nullptr;

    Unbind(wxEVT_IDLE, &cMain::OnIdle, this);
    Unbind(wxEVT_END_PROCESS, &cMain::OnProcessTerminated, this);
}

void cMain::OnCancelBuild(wxCommandEvent&)
{
    if (m_process)
    {
        long pid = m_process->GetPid();
        if (pid > 0)
        {
            // Kill the process and all its children
            wxString killCmd = wxString::Format("taskkill /PID %ld /T /F", pid);
            wxExecute(killCmd, wxEXEC_ASYNC);

            ConsoleOutput->AppendText("\nBuild cancelled by user.\n");
        }
    }
}

wxString cMain::GetTimeAgoString(const wxDateTime& lastBuilt)
{
    if (!lastBuilt.IsValid())
    {
        return "Never built";
    }

    wxTimeSpan diff = wxDateTime::Now() - lastBuilt;

    if (diff.GetMinutes() < 1)
        return "Just now";
    else if (diff.GetMinutes() == 1)
        return "1 minute ago";
    else if (diff.GetMinutes() < 60)
        return wxString::Format("%d minutes ago", (int)diff.GetMinutes());
    else if (diff.GetHours() < 24)
        return wxString::Format("%d hours ago", (int)diff.GetHours());
    else
        return wxString::Format("%d days ago", (int)diff.GetDays());
}

void cMain::OnUpdateTimer(wxTimerEvent& event)
{
    for(Entry& e : Entries)
    {
		e.UpdateLastBuiltText(this);
	}
}

void Entry::UpdateLastBuiltText(cMain* parent)
{
    if (LastBuiltText)
    {
        wxString timeAgo = "Never built";
        if (LastTimeBuilt.IsValid())
        {
            timeAgo = parent->GetTimeAgoString(LastTimeBuilt);
        }
        LastBuiltText->SetLabel(timeAgo);
    }
}

void ToDO::ScanTodosInDir(const wxString& dirPath, std::vector<TodoEntry>& outEntries)
{
    wxDir dir(dirPath);
    if (!dir.IsOpened())
        return;


    wxString filename;
    bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
    while (cont)
    {
        wxFileName fn(dirPath, filename);
        wxString ext = fn.GetExt().Lower();
        if (ext == "cpp" || ext == "h" || ext == "hpp" || ext == "c" || ext == "cs" || ext == "py" || ext == "m" || ext == "mm")
        {
            FindTodosInFile(fn.GetFullPath(), outEntries);
        }
        cont = dir.GetNext(&filename);
    }


    bool hasDir = dir.GetFirst(&filename, wxEmptyString, wxDIR_DIRS);
    while (hasDir)
    {
        // skip .git and build folders to speed up scan
        if (filename != ".git" && filename != "Binaries" && filename != "Intermediate" && filename != "Saved")
        {
            ScanTodosInDir(dirPath + wxFileName::GetPathSeparator() + filename, outEntries);
        }
        hasDir = dir.GetNext(&filename);
    }
}


void ToDO::FindTodosInFile(const wxString& filePath, std::vector<TodoEntry>& outEntries)
{
    wxTextFile file(filePath);
    if (!file.Exists() || !file.Open())
        return;

    const int lines = (int)file.GetLineCount();
    for (int i = 0; i < lines; ++i)
    {
        wxString line = file.GetLine(i).Trim(true).Trim(false);
        if (line.IsEmpty())
            continue;

        int pos = 0;
        while (pos < line.Length() && (line[pos] == '/' || wxIsspace(line[pos])))
            ++pos;

        if (pos >= line.Length())
            continue;

        wxString content = line.Mid(pos).Trim(true).Trim(false);

        int spacePos = content.Find(' ');
        wxString firstWord;
        if (spacePos == wxNOT_FOUND)
            firstWord = content;
        else
            firstWord = content.Left(spacePos);

        firstWord = firstWord.Upper();

        if (firstWord.EndsWith(":") || firstWord.EndsWith(","))
            firstWord = firstWord.Left(firstWord.Length() - 1);

        if (firstWord.StartsWith("[") && firstWord.Contains("]"))
        {
            int endBracket = firstWord.Find("]");
            firstWord = firstWord.Mid(0, endBracket + 1);
        }

        ///@TODO : we need a way to also add those dynamically so users can have custom tags not just mine
        wxString type;
        if (firstWord == "@TODO")
            type = "TODO";
        else if (firstWord == "@BUG")
            type = "BUG";
        else if (firstWord == "@NOTE")
            type = "NOTE";
        else if (firstWord == "[DEPRECATED]")
            type = "DEPRECATED";
        else
            continue;

     
        int start = wxMax(0, i - 2);
        int end = wxMin(lines - 1, i + 2);
        wxString ctx;
        for (int j = start; j <= end; ++j)
        {
            if(!file.GetLine(j).IsEmpty())
              ctx += wxString::Format("%4d: ", j + 1) + file.GetLine(j) + "\n";
        }

        TodoEntry t;
        t.File = filePath;
        t.Line = i + 1;
        t.Context = ctx;
        t.Type = type;

        outEntries.push_back(t);
    }
   
    //const int lines = (int)file.GetLineCount();
    //for (int i = 0; i < lines; ++i)
    //{
    //    wxString line = file.GetLine(i);
    //    wxString upper = line.Upper();
    //    if (upper.Find("@TODO") != wxNOT_FOUND || upper.Find("@BUG") != wxNOT_FOUND || upper.Find("@NOTE") != wxNOT_FOUND || upper.Find("[DEPRECATED]") != wxNOT_FOUND)
    //    {
    //        int start = wxMax(0, i - 2);
    //        int end = wxMin(lines - 1, i + 2);
    //        wxString ctx;
    //        for (int j = start; j <= end; ++j)
    //        {
    //            ctx += wxString::Format("%4d: ", j + 1) + file.GetLine(j) + "\n";
    //        }


    //        TodoEntry t;
    //        t.File = filePath;
    //        t.Line = i + 1;
    //        t.Context = ctx;

    //        if (upper.Find("[DEPRECATED]"))
    //            t.Type = "DEPRECATED";

    //        else
    //        {
    //            upper.erase(0);
    //            t.Type = upper;
    //        }

    //        outEntries.push_back(t);
    //    }
    //}
}

void cMain::OnShowTodoButton(wxCommandEvent& WXUNUSED(event))
{
    bool isTodoVisible = TodoPanel->IsShown();
    if (isTodoVisible)
    {
        // show console
        TodoPanel->Hide();
        ConsoleOutput->Show();
        ShowTodoButton->SetLabel("Show TODOs");
    }
    else
    {
        // show todo panel
        ConsoleOutput->Hide();
        TodoPanel->Show();
        ShowTodoButton->SetLabel("Show Console");
    }
    Layout();
}


void ToDO::OnTodoActivated(wxListEvent& event)
{
    long index = event.GetIndex();
    wxString file = TodoList->GetItemText(index, 0);
    wxString lineStr = TodoList->GetItemText(index, 1);
   // bool bIsOpenedWithDefaultEditor = false;

    //// attempt to open the file in default application. We cannot open at specific line reliably across editors,
    //// but user can open and navigate to the line.
    //if (wxFileExists(file))
    //{
    //    bIsOpenedWithDefaultEditor = wxLaunchDefaultApplication(file);
    //}

    //if (bIsOpenedWithDefaultEditor) return;

    long line = 1;
    lineStr.ToLong(&line);

    if (!wxFileExists(file))
        return;

    wxString fixed = file;
    fixed.Replace("\\", "/");

    //wxString cmd = wxString::Format("code -g \"%s\":%ld", fixed, line);
    wxString cmd = wxString::Format("cmd /c code -g \"%s\":%ld", fixed, line);
    wxExecute(cmd, wxEXEC_ASYNC | wxEXEC_HIDE_CONSOLE);
}

ToDO::ToDO(wxWindow* parent, Entry& entry)
    : wxDialog(parent, wxID_ANY, "Source Code analyse", wxDefaultPosition, wxSize(1280, 720))
{

    TodoPanel = new wxPanel(this, wxID_ANY);

    wxBoxSizer* todoSizer = new wxBoxSizer(wxVERTICAL);
    TodoDirPicker = new wxDirPickerCtrl(TodoPanel, wxID_ANY, "", "Select source directory");
    TodoScanButton = new wxButton(TodoPanel, wxID_ANY, "Scan Source Code");
    TodoList = new wxListCtrl(TodoPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_SINGLE_SEL);


    TodoList->InsertColumn(0, "File", wxLIST_FORMAT_LEFT, 400);
    TodoList->InsertColumn(1, "Line", wxLIST_FORMAT_LEFT, 60);
    TodoList->InsertColumn(2, "Context", wxLIST_FORMAT_LEFT, 700);
    TodoList->InsertColumn(3, "Type", wxLIST_FORMAT_LEFT, 80);

    todoSizer->Add(TodoDirPicker, 0, wxEXPAND | wxALL, 5);
    todoSizer->Add(TodoScanButton, 0, wxEXPAND | wxALL, 5);
    todoSizer->Add(TodoList, 1, wxEXPAND | wxALL, 5);


    TodoPanel->SetSizer(todoSizer);

    TodoDirPicker->SetPath(ReplaceLastWithSource(entry.Path));

    CallAfter([this]()
        {
            if (auto txt = TodoDirPicker->GetTextCtrl())
            {
                txt->ShowPosition(0);
                txt->SetInsertionPoint(0);
                txt->SelectNone();
            }
        });

    // Bind TODO scan
    TodoScanButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
        {
            wxString dir = TodoDirPicker->GetPath();
            if (dir.IsEmpty())
            {
                wxMessageBox("Please select a source directory first.");
                return;
            }


            // Busy cursor
            wxBeginBusyCursor();
            std::vector<TodoEntry> todos;
            ScanTodosInDir(dir, todos);
            wxEndBusyCursor();


            TodoList->DeleteAllItems();
            for (size_t i = 0; i < todos.size(); ++i)
            {
                long idx = TodoList->InsertItem((long)i, todos[i].File);
                TodoList->SetItem(idx, 1, wxString::Format("%ld", todos[i].Line));
                TodoList->SetItem(idx, 2, todos[i].Context);
                TodoList->SetItem(idx, 3, todos[i].Type);
            }


            wxMessageBox(wxString::Format("Found %zu Developer comments", todos.size()));
        });

    static long lastTooltipItem = -1;

    TodoList->Bind(wxEVT_MOTION, [this](wxMouseEvent& event)
        {
            wxPoint pt = event.GetPosition();
            int flags;
            long item = TodoList->HitTest(pt, flags);

            int x = pt.x;
            int col0Width = TodoList->GetColumnWidth(0);
            int col1Width = TodoList->GetColumnWidth(1);
            int col2Width = TodoList->GetColumnWidth(2);

            int col2Start = col0Width + col1Width;
            int col2End = col2Start + col2Width;

            if (item != wxNOT_FOUND && (flags & wxLIST_HITTEST_ONITEM) && x >= col2Start && x < col2End)
            {
                if (lastTooltipItem != item)
                {
                    wxString contextText = TodoList->GetItemText(item, 2);
                    TodoList->SetToolTip(contextText);
                    lastTooltipItem = item;
                }
            }
            else
            {

                if (lastTooltipItem != -1)
                {
                    TodoList->UnsetToolTip();
                    lastTooltipItem = -1;
                }
            }

            event.Skip();
        });

    // double click to open file
    TodoList->Bind(wxEVT_LIST_ITEM_ACTIVATED, &ToDO::OnTodoActivated, this);
}
