#include "wxSceneInspector.h"

#include <iomanip>
#include <sstream>
#include <vector>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/treectrl.h>

namespace
{
enum class InspectorItemKind { Root, Chunk, Object, Diagnostics };

class InspectorItemData final : public wxTreeItemData
{
public:
    InspectorItemData(InspectorItemKind kind, std::size_t index = 0) :
        kind(kind), index(index)
    {
    }
    InspectorItemKind kind;
    std::size_t index;
};

std::string Hex(std::uint32_t value)
{
    std::ostringstream output;
    output << "0x" << std::uppercase << std::hex << std::setw(8)
        << std::setfill('0') << value;
    return output.str();
}
}

wxSceneInspector::wxSceneInspector(wxWindow* parent) : wxPanel(parent)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    summary_ = new wxStaticText(this, wxID_ANY,
        "No historical scene inspected.");
    tree_ = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_SINGLE);
    details_ = new wxTextCtrl(this, wxID_ANY,
        "Inspection is read-only and session-local.", wxDefaultPosition,
        wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    sizer->Add(summary_, 0, wxEXPAND | wxALL, 6);
    sizer->Add(tree_, 3, wxEXPAND | wxLEFT | wxRIGHT, 6);
    sizer->Add(details_, 2, wxEXPAND | wxALL, 6);
    SetSizer(sizer);
    tree_->Bind(wxEVT_TREE_SEL_CHANGED,
        &wxSceneInspector::OnSelectionChanged, this);
}

void wxSceneInspector::ClearManifest()
{
    manifest_ = {};
    hasManifest_ = false;
    summary_->SetLabel("No historical scene inspected.");
    tree_->DeleteAllItems();
    details_->SetValue("Inspection is read-only and session-local.");
}

void wxSceneInspector::SetManifest(const EditorSceneManifest& manifest)
{
    manifest_ = manifest;
    hasManifest_ = true;
    summary_->SetLabel(wxString::Format(
        "%s | %zu bytes | %zu chunks | %zu objects | compressed %zu/%zu",
        wxString::FromUTF8(manifest_.format), manifest_.totalSize,
        manifest_.chunks.size(), manifest_.objects.size(),
        manifest_.decompressedChunkCount, manifest_.compressedChunkCount));
    tree_->DeleteAllItems();
    const wxTreeItemId root = tree_->AddRoot(
        wxString::FromUTF8(manifest_.sourceFile), -1, -1,
        new InspectorItemData(InspectorItemKind::Root));
    std::vector<wxTreeItemId> parents{root};
    for (std::size_t index = 0; index < manifest_.chunks.size(); ++index)
    {
        const EditorSceneChunkRecord& chunk = manifest_.chunks[index];
        while (parents.size() > chunk.depth + 1)
            parents.pop_back();
        const wxTreeItemId parent = parents.empty() ? root : parents.back();
        std::string label = Hex(chunk.id) + " " + chunk.label + " (" +
            std::to_string(chunk.size) + " bytes)";
        if (chunk.compressed)
            label += chunk.decompressionSucceeded ? " [decoded]" : " [compressed]";
        const wxTreeItemId item = tree_->AppendItem(parent,
            wxString::FromUTF8(label), -1, -1,
            new InspectorItemData(InspectorItemKind::Chunk, index));
        if (parents.size() == chunk.depth + 1)
            parents.push_back(item);
        else
            parents[chunk.depth + 1] = item;
    }

    const wxTreeItemId objects = tree_->AppendItem(root, "Confirmed Objects",
        -1, -1, new InspectorItemData(InspectorItemKind::Root));
    for (std::size_t index = 0; index < manifest_.objects.size(); ++index)
    {
        const EditorSceneObjectRecord& object = manifest_.objects[index];
        std::string label = "#" + std::to_string(object.recordIndex) +
            " class " + std::to_string(object.classId);
        if (object.hasName)
            label += " " + object.name;
        tree_->AppendItem(objects, wxString::FromUTF8(label), -1, -1,
            new InspectorItemData(InspectorItemKind::Object, index));
    }
    tree_->AppendItem(root, wxString::Format("Diagnostics (%zu)",
        manifest_.diagnostics.size()), -1, -1,
        new InspectorItemData(InspectorItemKind::Diagnostics));
    tree_->Expand(root);
    tree_->Expand(objects);
    tree_->SelectItem(root);
}

void wxSceneInspector::OnSelectionChanged(wxTreeEvent& event)
{
    const auto* data = dynamic_cast<InspectorItemData*>(
        tree_->GetItemData(event.GetItem()));
    if (!data || !hasManifest_)
        return;
    std::ostringstream output;
    if (data->kind == InspectorItemKind::Root)
    {
        output << "Source: " << manifest_.sourceFile << '\n'
            << "Format: " << manifest_.format << '\n'
            << "Version: " << manifest_.version << '\n'
            << "Declared objects: ";
        if (manifest_.hasDeclaredObjectCount)
            output << manifest_.declaredObjectCount;
        else
            output << "not present";
        output << "\nConfirmed objects: " << manifest_.objects.size()
            << "\nUnknown chunks: " << manifest_.unknownChunkCount
            << "\nCompressed chunks: " << manifest_.compressedChunkCount
            << "\nDecompressed successfully: "
            << manifest_.decompressedChunkCount
            << "\nDecompression failures: "
            << manifest_.decompressionFailureCount
            << "\nCompressed bytes: " << manifest_.totalCompressedBytes
            << "\nDecompressed bytes: " << manifest_.totalDecompressedBytes
            << "\nCompression algorithm: "
            << (manifest_.compressionAlgorithm.empty()
                ? "not encountered" : manifest_.compressionAlgorithm);
    }
    else if (data->kind == InspectorItemKind::Chunk &&
        data->index < manifest_.chunks.size())
    {
        const EditorSceneChunkRecord& chunk = manifest_.chunks[data->index];
        output << "Path: " << chunk.path << '\n'
            << "Label: " << chunk.label << '\n'
            << "Header offset: " << chunk.headerOffset << '\n'
            << "Data offset: " << chunk.dataOffset << '\n'
            << "Payload size: " << chunk.size << '\n'
            << "Compressed flag: " << (chunk.compressed ? "yes" : "no");
        if (chunk.compressed)
        {
            output << "\nCompression algorithm: "
                << chunk.compressionAlgorithm
                << "\nCompressed bytes: " << chunk.compressedSize
                << "\nDecompressed bytes: " << chunk.decompressedSize
                << "\nDecompression supported: "
                << (chunk.decompressionSupported ? "yes" : "no")
                << "\nDecompression succeeded: "
                << (chunk.decompressionSucceeded ? "yes" : "no")
                << "\nDiagnostic: "
                << (chunk.compressionDiagnostic.empty()
                    ? "none" : chunk.compressionDiagnostic);
        }
        if (chunk.fromDecompressedPayload)
            output << "\nCompressed source offset: "
                << chunk.compressedSourceOffset
                << "\nDecoded stream offset: " << chunk.decompressedOffset;
    }
    else if (data->kind == InspectorItemKind::Object &&
        data->index < manifest_.objects.size())
    {
        const EditorSceneObjectRecord& object = manifest_.objects[data->index];
        output << "Record index: " << object.recordIndex << '\n'
            << "Class ID: " << object.classId << '\n'
            << "Name: " << (object.hasName ? object.name : "not present")
            << "\nSource offset: " << object.sourceOffset << '\n'
            << "Chunk path: " << object.chunkPath;
        if (object.fromDecompressedPayload)
            output << "\nCompressed source offset: "
                << object.compressedSourceOffset
                << "\nDecoded stream offset: " << object.decompressedOffset;
        if (object.hasTransform)
        {
            output << "\nPosition: " << object.position[0] << ", "
                << object.position[1] << ", " << object.position[2]
                << "\nRotation: " << object.rotation[0] << ", "
                << object.rotation[1] << ", " << object.rotation[2]
                << "\nScale: " << object.scale[0] << ", "
                << object.scale[1] << ", " << object.scale[2];
        }
        else
            output << "\nTransform: not present";
    }
    else if (data->kind == InspectorItemKind::Diagnostics)
    {
        for (const EditorSceneDiagnostic& diagnostic : manifest_.diagnostics)
            output << "offset " << diagnostic.offset << ": "
                << diagnostic.message << '\n';
        if (manifest_.diagnostics.empty())
            output << "No retained diagnostics.";
    }
    details_->SetValue(wxString::FromUTF8(output.str()));
}
