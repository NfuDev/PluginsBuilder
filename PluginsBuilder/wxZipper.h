#pragma once

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/filename.h>


#include <wx/filesys.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#include <wx/listctrl.h>
#include <wx/gbsizer.h>
#include <wx/progdlg.h>

struct IgnoringTraverser : public wxDirTraverser
{
    std::function<bool(const std::string&)> shouldIgnoreDir;
    std::function<void(const std::string&)> fileEnterCallback;

    virtual wxDirTraverseResult OnFile(const wxString& filename) override
    {
        fileEnterCallback(filename.ToStdString());
        return wxDIR_CONTINUE;
    }

    virtual wxDirTraverseResult OnDir(const wxString& dirname) override
    {
        auto lastPathComponent = wxFileName(dirname).GetFullName();

        return shouldIgnoreDir(lastPathComponent.ToStdString()) ? wxDIR_IGNORE : wxDIR_CONTINUE;
    }
};

class wxZipper
{
public:

    wxString dirToCompressText;
    wxString zipFileText;
    wxFrame* HostPanel;
    std::vector<wxString> filesList;

    void PreparePluginToCompress(wxString& BuildPath, wxString& CompressedPath)
    {
        dirToCompressText = BuildPath;
        zipFileText = CompressedPath;

        static constexpr int PulseInterval = 100;

        if (!wxDirExists(dirToCompressText))
        {
            return;
        }

        wxProgressDialog dialog("Loading files", "Loading files to compress...", 100, HostPanel, wxPD_APP_MODAL | wxPD_AUTO_HIDE);

            IgnoringTraverser traverser;
            traverser.shouldIgnoreDir = [this](const std::string& dirName)
                {
                    return dirName == "Intermediate" || dirName == "Binaries";
                };

            traverser.fileEnterCallback = [this, &dialog](const std::string& fileName)
                {
                    auto itemCount = filesList.size();

                    if (itemCount % PulseInterval == 0)
                    {
                        dialog.Pulse();
                    }

                    filesList.push_back(fileName);
                };

            wxDir(dirToCompressText).Traverse(traverser);
 
    }

    void PerformCompression()
    {
        wxFileOutputStream outStream(zipFileText);

        if (!outStream.IsOk())
        {
            wxMessageBox("Failed to open zip file", "Error", wxOK | wxICON_ERROR);
            return;
        }

        wxZipOutputStream zip(outStream);

        wxProgressDialog dialog("Compressing", "Compressing files...", filesList.size(), HostPanel, wxPD_APP_MODAL | wxPD_AUTO_HIDE);

        for (int i = 0; i < filesList.size(); i++)
        {
            auto file = filesList[i];

            wxFileName fileName(file);
            fileName.MakeRelativeTo(dirToCompressText);

            auto relativePath = fileName.GetFullPath(wxPATH_NATIVE);

            wxLogDebug("Compressing %s as %s", file, relativePath);

            zip.PutNextEntry(relativePath);
            wxFFileInputStream(file).Read(zip);

            zip.CloseEntry();

            dialog.Update(i);
        }

        zip.Close();
        outStream.Close();
    }
};