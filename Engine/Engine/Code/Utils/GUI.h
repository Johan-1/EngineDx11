#pragma once
#include "imgui.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <Windows.h>
#include <string>

using namespace DirectX;

namespace GUI
{
	static void BeginWindow(const char* name, float alpha, float sizeX, float sizeY, float posX, float posY, float pivX, float pivY)
	{
		// set properties of next window;
		ImGui::SetNextWindowBgAlpha(alpha);
		ImGui::SetNextWindowSize(ImVec2(sizeX, sizeY));
		ImGui::SetNextWindowPos(ImVec2(posX, posY), 0, ImVec2(pivX, pivY));

		// create window to display information
		ImGui::Begin(name, nullptr);
	}

	static void BeginWindowBoarderless(const char* name, float alpha, float sizeX, float sizeY, float posX, float posY, float pivX, float pivY)
	{
		// set properties of next window;
		ImGui::SetNextWindowBgAlpha(alpha);
		ImGui::SetNextWindowSize(ImVec2(sizeX, sizeY));
		ImGui::SetNextWindowPos(ImVec2(posX, posY), 0, ImVec2(pivX, pivY));

		// create window to display information
		ImGui::Begin(name, nullptr, ImGuiWindowFlags_NoTitleBar);
	}

	static bool Button(const char* text)
	{
		return ImGui::Button(text);
	}

	static void EndWindow()
	{
		ImGui::End();
	}

	static void Text(ImVec4 color, const char* text)
	{
		ImGui::TextColored(color, text);
	}

	static void ShowToolTip(const char* tip)
	{
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::SetTooltip(tip);
			ImGui::EndTooltip();
		}
	}
	
	static void ColorPickerNoAlpha(const char* ID, const char* header, const char* toolTip, const char* pickerName, XMFLOAT4* data)
	{
		if (ImGui::ColorButton(ID, ImVec4(data->x, data->y, data->z, 0), ImGuiColorEditFlags_NoAlpha))
			ImGui::OpenPopup(pickerName);
	
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), header);
		ShowToolTip(toolTip);
	
		if (ImGui::BeginPopup(pickerName))
		{
			ImGui::ColorPicker4(pickerName, (float*)data, ImGuiColorEditFlags_NoAlpha);
			ImGui::EndPopup();
		}
	}

	static void ColorPickerNoAlpha(const char* ID, const char* header, const char* toolTip, const char* pickerName, XMFLOAT3* data)
	{
		if (ImGui::ColorButton(ID, ImVec4(data->x, data->y, data->z, 0), ImGuiColorEditFlags_NoAlpha))
			ImGui::OpenPopup(pickerName);

		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), header);
		ShowToolTip(toolTip);

		if (ImGui::BeginPopup(pickerName))
		{
			ImGui::ColorPicker4(pickerName, (float*)data, ImGuiColorEditFlags_NoAlpha);
			ImGui::EndPopup();
		}
	}
	
	static void FloatSlider(const char* ID, const char* header, const char* toolTip, float min, float max, float* data)
	{
		ImGui::SliderFloat(ID, data, min, max, "%.2f");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), header);
		ShowToolTip(toolTip);
	}
	
	static void FloatSlider2(const char* ID, const char* header, const char* toolTip, float min, float max, float* data)
	{
		ImGui::SliderFloat2(ID, data, min, max, "%.2f");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), header);
		ShowToolTip(toolTip);
	}
	
	static void FloatInput3(const char* ID, const char* header, const char* toolTip, float* data)
	{
		ImGui::InputFloat3(ID, data, 2);
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), header);
		ShowToolTip(toolTip);
	}

	static void IntInput2(const char* ID, const char* header, const char* toolTip, int* data)
	{
		ImGui::InputInt2(ID, data);
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), header);
		ShowToolTip(toolTip);
	}
	
	static void FloatInput(const char* ID, const char* header, const char* toolTip, float* data)
	{
		ImGui::InputFloat(ID, data, 1.0f, 1.0f, 1);
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), header);
		ShowToolTip(toolTip);
	}

	static void SetItemWidth(float width)
	{
		ImGui::PushItemWidth(width);
	}

	static void ClearItemWidth()
	{
		ImGui::PopItemWidth();
	}

	static void Space(float height)
	{
		ImGui::Spacing(height);
	}

	static void SameLine()
	{
		ImGui::SameLine();
	}

	static void Image(ID3D11ShaderResourceView* texture, ImVec2 size)
	{
		ImGui::Image((void*)texture, size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
	}

	static void ComboBox(const char* ID, const char* header, const char* toolTip, int* index, const char* items)
	{
		ImGui::Combo(ID, index, items);
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), header);
		ShowToolTip(toolTip);
	}

	static void CheckBox(const char* ID, const char* header, const char* toolTip, bool* data)
	{
		ImGui::Checkbox(ID, data);
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), header);
		ShowToolTip(toolTip);
	}

	static void ComboBox(const char* ID, const char* header, const char* toolTip, const char* items, int* data)
	{
		ImGui::Combo(ID, data, items);
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), header);
		ShowToolTip(toolTip);
	}

	static std::string FindFileFromDirectory(const char* filter, const char* title)
	{
		// create memory for the path to the texture file we select
		char filename[MAX_PATH];
		ZeroMemory(&filename, sizeof(filename));

		// create memory for the path to our solution directory
		char oldDir[MAX_PATH];
		ZeroMemory(&oldDir, sizeof(oldDir));

		// create settings for the open file directory window
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner   = NULL;
		ofn.lpstrFilter = filter;
		ofn.lpstrFile   = filename;
		ofn.nMaxFile    = MAX_PATH;
		ofn.lpstrTitle  = title;
		ofn.Flags       = 0;
		ofn.lpstrDefExt = ".json";

		// we have to save the current directory before we open the openfile directory so we can set it back efter we have selected a file
		// it will permanantly change our base directory making searches from $SolutionDir/ not work anymore if we not
		GetCurrentDirectory(MAX_PATH, oldDir);

		if (GetOpenFileName(&ofn))
		{
			// set back to old directory
			SetCurrentDirectory(oldDir);

			return filename;
		}

		return "";
	}

	static std::string ReplaceWithRelativePath(std::string relPath, std::string path)
	{
		// get the offset where our last backslash or forwardslash is located
		size_t lastSlash = path.find_last_of("\\");
		if (std::string::npos == lastSlash)
			lastSlash = path.find_last_of("/");

		// erease filepath
		if (std::string::npos != lastSlash)
			path.erase(0, lastSlash + 1);

		// add filename to relative path		
		return relPath.append(path.c_str());
	}
}