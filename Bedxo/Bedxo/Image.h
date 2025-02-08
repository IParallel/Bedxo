#pragma once
#include <D3DX11tex.h>
#include <imgui.h>
namespace Bedxo
{
	class Image
	{
	public:
		Image(ID3D11ShaderResourceView* shader, ImVec2 size) : m_Texture(shader), m_Size(size) {};
		Image() {};
		~Image() { m_Texture->Release(); };
		ImVec2 GetSize() const { return m_Size; }
		ImTextureID GetTexture() { return (ImTextureID)(intptr_t)m_Texture; }
	private:
		ID3D11ShaderResourceView* m_Texture = nullptr;
		ImVec2 m_Size;
	};
}

