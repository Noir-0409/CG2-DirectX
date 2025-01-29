#include <Windows.h>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dxgidebug.h>
#include "externals/DirectXTex/DirectXTex.h"
#include <fstream>
#include <sstream>
#include <wrl.h>
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "StringUtility.h"
#include "externals/imgui/imgui_impl_win32.h"


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

struct ModelData {

	std::vector<VertexData> vertices;
	MaterialData material;

};

//Transform変数の作成
Transform transform{

	{1.0f,1.0f,1.0f},
	{0.0f,3.150f,0.0f},
	{0.0f,0.0f,0.0f}

};

Transform transformSprite{

	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f}

};

Transform cameraTransform{

	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,-10.0f}

};

Transform uvTransformSprite{

	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f}

};

void Log(const std::string& message) {

	OutputDebugStringA(message.c_str());

}

//単位行列の作成
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result;

	// 対角線上の要素を1に設定し、それ以外の要素を0に設定する
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (i == j) {
				result.m[i][j] = 1.0f;
			} else {
				result.m[i][j] = 0.0f;
			}
		}
	}

	return result;

}

//平行移動行列
Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {

	Matrix4x4 translateMatrix;

	// 平行移動行列の生成
	translateMatrix.m[0][0] = 1.0f;
	translateMatrix.m[0][1] = 0.0f;
	translateMatrix.m[0][2] = 0.0f;
	translateMatrix.m[0][3] = 0.0f;

	translateMatrix.m[1][0] = 0.0f;
	translateMatrix.m[1][1] = 1.0f;
	translateMatrix.m[1][2] = 0.0f;
	translateMatrix.m[1][3] = 0.0f;

	translateMatrix.m[2][0] = 0.0f;
	translateMatrix.m[2][1] = 0.0f;
	translateMatrix.m[2][2] = 1.0f;
	translateMatrix.m[2][3] = 0.0f;

	translateMatrix.m[3][0] = translate.x;
	translateMatrix.m[3][1] = translate.y;
	translateMatrix.m[3][2] = translate.z;
	translateMatrix.m[3][3] = 1.0f;

	return translateMatrix;

}

//拡大縮小行列
Matrix4x4 MakeScaleMatrix(const Vector3& scale) {

	Matrix4x4 scaleMatrix;

	// 拡大縮小行列の生成
	scaleMatrix.m[0][0] = scale.x;
	scaleMatrix.m[0][1] = 0.0f;
	scaleMatrix.m[0][2] = 0.0f;
	scaleMatrix.m[0][3] = 0.0f;

	scaleMatrix.m[1][0] = 0.0f;
	scaleMatrix.m[1][1] = scale.y;
	scaleMatrix.m[1][2] = 0.0f;
	scaleMatrix.m[1][3] = 0.0f;

	scaleMatrix.m[2][0] = 0.0f;
	scaleMatrix.m[2][1] = 0.0f;
	scaleMatrix.m[2][2] = scale.z;
	scaleMatrix.m[2][3] = 0.0f;

	scaleMatrix.m[3][0] = 0.0f;
	scaleMatrix.m[3][1] = 0.0f;
	scaleMatrix.m[3][2] = 0.0f;
	scaleMatrix.m[3][3] = 1.0f;

	return scaleMatrix;
}

//X軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian) {

	Matrix4x4 rotateXMatrix;

	float cosTheta = std::cos(radian);
	float sinTheta = std::sin(radian);

	// X軸周りの回転行列の生成
	rotateXMatrix.m[0][0] = 1.0f;
	rotateXMatrix.m[0][1] = 0.0f;
	rotateXMatrix.m[0][2] = 0.0f;
	rotateXMatrix.m[0][3] = 0.0f;

	rotateXMatrix.m[1][0] = 0.0f;
	rotateXMatrix.m[1][1] = cosTheta;
	rotateXMatrix.m[1][2] = sinTheta;
	rotateXMatrix.m[1][3] = 0.0f;

	rotateXMatrix.m[2][0] = 0.0f;
	rotateXMatrix.m[2][1] = -sinTheta;
	rotateXMatrix.m[2][2] = cosTheta;
	rotateXMatrix.m[2][3] = 0.0f;

	rotateXMatrix.m[3][0] = 0.0f;
	rotateXMatrix.m[3][1] = 0.0f;
	rotateXMatrix.m[3][2] = 0.0f;
	rotateXMatrix.m[3][3] = 1.0f;

	return rotateXMatrix;

}

//Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian) {

	Matrix4x4 rotateYMatrix;

	float cosTheta = std::cos(radian);
	float sinTheta = std::sin(radian);

	// Y軸周りの回転行列の生成
	rotateYMatrix.m[0][0] = cosTheta;
	rotateYMatrix.m[0][1] = 0.0f;
	rotateYMatrix.m[0][2] = -sinTheta;
	rotateYMatrix.m[0][3] = 0.0f;

	rotateYMatrix.m[1][0] = 0.0f;
	rotateYMatrix.m[1][1] = 1.0f;
	rotateYMatrix.m[1][2] = 0.0f;
	rotateYMatrix.m[1][3] = 0.0f;

	rotateYMatrix.m[2][0] = sinTheta;
	rotateYMatrix.m[2][1] = 0.0f;
	rotateYMatrix.m[2][2] = cosTheta;
	rotateYMatrix.m[2][3] = 0.0f;

	rotateYMatrix.m[3][0] = 0.0f;
	rotateYMatrix.m[3][1] = 0.0f;
	rotateYMatrix.m[3][2] = 0.0f;
	rotateYMatrix.m[3][3] = 1.0f;

	return rotateYMatrix;

}

//Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian) {

	Matrix4x4 rotateZMatrix;

	float cosTheta = std::cos(radian);
	float sinTheta = std::sin(radian);

	// Z軸周りの回転行列の生成
	rotateZMatrix.m[0][0] = cosTheta;
	rotateZMatrix.m[0][1] = sinTheta;
	rotateZMatrix.m[0][2] = 0.0f;
	rotateZMatrix.m[0][3] = 0.0f;

	rotateZMatrix.m[1][0] = -sinTheta;
	rotateZMatrix.m[1][1] = cosTheta;
	rotateZMatrix.m[1][2] = 0.0f;
	rotateZMatrix.m[1][3] = 0.0f;

	rotateZMatrix.m[2][0] = 0.0f;
	rotateZMatrix.m[2][1] = 0.0f;
	rotateZMatrix.m[2][2] = 1.0f;
	rotateZMatrix.m[2][3] = 0.0f;

	rotateZMatrix.m[3][0] = 0.0f;
	rotateZMatrix.m[3][1] = 0.0f;
	rotateZMatrix.m[3][2] = 0.0f;
	rotateZMatrix.m[3][3] = 1.0f;

	return rotateZMatrix;

}

// 行列の積
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {

	Matrix4x4 result;

	// 行列の各要素について、行列の積を計算する
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = 0; // 初期化しておく
			for (int k = 0; k < 4; ++k) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}

	// 結果の行列を返す
	return result;

}

//3次元アフィン変換行列
Matrix4x4 MakeAffinMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {

	// スケーリング行列の作成
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);

	// X軸回転行列の作成
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);

	// Y軸回転行列の作成
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);

	// Z軸回転行列の作成
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);

	// 平行移動行列の作成
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

	// スケーリング行列とX軸回転行列を乗算
	Matrix4x4 result = Multiply(scaleMatrix, rotateXMatrix);

	// Y軸回転行列を乗算
	result = Multiply(result, rotateYMatrix);

	// Z軸回転行列を乗算
	result = Multiply(result, rotateZMatrix);

	// 平行移動行列を乗算
	result = Multiply(result, translateMatrix);

	// 最終的なアフィン変換行列を返す
	return result;

}

//逆行列
Matrix4x4 Inverse(const Matrix4x4& m) {
	Matrix4x4 result;

	// 行列の余因子行列を計算
	result.m[0][0] = m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[1][3] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][3] * m.m[3][2] - m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[1][3] * m.m[2][2] * m.m[3][1];
	result.m[0][1] = m.m[0][1] * m.m[2][3] * m.m[3][2] + m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][3] * m.m[2][2] * m.m[3][1] - m.m[0][1] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[2][1] * m.m[3][2];
	result.m[0][2] = m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][3] * m.m[3][2] - m.m[0][2] * m.m[1][1] * m.m[3][3] - m.m[0][3] * m.m[1][2] * m.m[3][1];
	result.m[0][3] = m.m[0][1] * m.m[1][3] * m.m[2][2] + m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][3] * m.m[1][2] * m.m[2][1] - m.m[0][1] * m.m[1][2] * m.m[2][3] - m.m[0][2] * m.m[1][3] * m.m[2][1] - m.m[0][3] * m.m[1][1] * m.m[2][2];

	result.m[1][0] = m.m[1][0] * m.m[2][3] * m.m[3][2] + m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[1][3] * m.m[2][2] * m.m[3][0] - m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[1][3] * m.m[2][0] * m.m[3][2];
	result.m[1][1] = m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] + m.m[0][3] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][3] * m.m[3][2] - m.m[0][2] * m.m[2][0] * m.m[3][3] - m.m[0][3] * m.m[2][2] * m.m[3][0];
	result.m[1][2] = m.m[0][0] * m.m[1][3] * m.m[3][2] + m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][3] * m.m[1][2] * m.m[3][0] - m.m[0][0] * m.m[1][2] * m.m[3][3] - m.m[0][2] * m.m[1][3] * m.m[3][0] - m.m[0][3] * m.m[1][0] * m.m[3][2];
	result.m[1][3] = m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] + m.m[0][3] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][3] * m.m[2][2] - m.m[0][2] * m.m[1][0] * m.m[2][3] - m.m[0][3] * m.m[1][2] * m.m[2][0];

	result.m[2][0] = m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[1][3] * m.m[2][0] * m.m[3][1] - m.m[1][0] * m.m[2][3] * m.m[3][1] - m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][3] * m.m[2][1] * m.m[3][0];
	result.m[2][1] = m.m[0][0] * m.m[2][3] * m.m[3][1] + m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][3] * m.m[2][1] * m.m[3][0] - m.m[0][0] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][0] - m.m[0][3] * m.m[2][0] * m.m[3][1];
	result.m[2][2] = m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] + m.m[0][3] * m.m[1][0] * m.m[3][1] - m.m[0][0] * m.m[1][3] * m.m[3][1] - m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][3] * m.m[1][1] * m.m[3][0];
	result.m[2][3] = m.m[0][0] * m.m[1][3] * m.m[2][1] + m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][3] * m.m[1][1] * m.m[2][0] - m.m[0][0] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] - m.m[0][3] * m.m[1][0] * m.m[2][1];

	result.m[3][0] = m.m[1][0] * m.m[2][2] * m.m[3][1] + m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][2] * m.m[2][1] * m.m[3][0] - m.m[1][0] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][2] * m.m[3][0] - m.m[1][2] * m.m[2][0] * m.m[3][1];
	result.m[3][1] = m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] + m.m[0][2] * m.m[2][0] * m.m[3][1] - m.m[0][0] * m.m[2][2] * m.m[3][1] - m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][2] * m.m[2][1] * m.m[3][0];
	result.m[3][2] = m.m[0][0] * m.m[1][2] * m.m[3][1] + m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][2] * m.m[1][1] * m.m[3][0] - m.m[0][0] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][2] * m.m[3][0] - m.m[0][2] * m.m[1][0] * m.m[3][1];
	result.m[3][3] = m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] + m.m[0][2] * m.m[1][0] * m.m[2][1] - m.m[0][0] * m.m[1][2] * m.m[2][1] - m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][2] * m.m[1][1] * m.m[2][0];

	// 行列式を計算
	float determinant = m.m[0][0] * result.m[0][0] + m.m[0][1] * result.m[1][0] + m.m[0][2] * result.m[2][0] + m.m[0][3] * result.m[3][0];

	// 行列式が0の場合、逆行列は存在しない
	if (determinant == 0) {

		return result; // ゼロ行列を返すことでエラーを示す
	}

	// 行列の逆行列を計算
	float inverseFactor = 1.0f / determinant;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] *= inverseFactor;
		}
	}

	return result;
}

//透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {

	float f = 1.0f / std::tan(fovY / 2.0f);
	Matrix4x4 perspectiveMatrix;

	perspectiveMatrix.m[0][0] = f / aspectRatio;
	perspectiveMatrix.m[0][1] = 0;
	perspectiveMatrix.m[0][2] = 0;
	perspectiveMatrix.m[0][3] = 0;

	perspectiveMatrix.m[1][0] = 0;
	perspectiveMatrix.m[1][1] = f;
	perspectiveMatrix.m[1][2] = 0;
	perspectiveMatrix.m[1][3] = 0;

	perspectiveMatrix.m[2][0] = 0;
	perspectiveMatrix.m[2][1] = 0;
	perspectiveMatrix.m[2][2] = farClip / (farClip - nearClip);
	perspectiveMatrix.m[2][3] = 1;

	perspectiveMatrix.m[3][0] = 0;
	perspectiveMatrix.m[3][1] = 0;
	perspectiveMatrix.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);
	perspectiveMatrix.m[3][3] = 0;

	return perspectiveMatrix;

}

//平行投影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {

	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	//float tz = -(farClip + nearClip) / (farClip - nearClip);

	Matrix4x4 orthoMatrix;

	orthoMatrix.m[0][0] = 2.0f / (right - left);
	orthoMatrix.m[0][1] = 0;
	orthoMatrix.m[0][2] = 0;
	orthoMatrix.m[0][3] = 0;

	orthoMatrix.m[1][0] = 0;
	orthoMatrix.m[1][1] = 2.0f / (top - bottom);
	orthoMatrix.m[1][2] = 0;
	orthoMatrix.m[1][3] = 0;

	orthoMatrix.m[2][0] = 0;
	orthoMatrix.m[2][1] = 0;
	orthoMatrix.m[2][2] = 1.0f / (farClip - nearClip);
	orthoMatrix.m[2][3] = 0;

	orthoMatrix.m[3][0] = tx;
	orthoMatrix.m[3][1] = ty;
	orthoMatrix.m[3][2] = -2.0f / (farClip - nearClip);
	orthoMatrix.m[3][3] = 1;

	return orthoMatrix;

}

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();

	handleCPU.ptr += (descriptorSize * index);

	return handleCPU;

}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();

	handleGPU.ptr += (descriptorSize * index);

	return handleGPU;

}


MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {

	// 中で必要になる変数の宣言
	MaterialData materialData;

	std::string line;

	// ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);

	assert(file.is_open());

	// 実際にファイルを読み、MaterialDataを構築
	while (std::getline(file, line)) {

		std::string identifier;

		std::istringstream s(line);

		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {

			std::string textureFilename;

			s >> textureFilename;

			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;

		}

	}

	// MaterialDataを返す
	return materialData;

}

ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {

	// 中で必要となる変数の宣言
	ModelData modelData; // 構築するModelData

	std::vector<Vector4> positions; // 位置

	std::vector<Vector3> normals; // 法線

	std::vector<Vector2> texcoords; // テクスチャ座標

	std::string line; // ファイルから読んだ1行を格納するもの

	// ファイルを開く
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く

	assert(file.is_open()); // 開けなかったら止める

	// 実際にファイルを読み、ModelDataを構築
	while (std::getline(file, line)) {

		std::string identifier;

		std::istringstream s(line);

		s >> identifier; // 先頭の識別子を読む

		// identifierに応じた処理
		if (identifier == "v") {

			Vector4 position;

			s >> position.x >> position.y >> position.z;

			position.x *= -1.0f;

			position.w = 1.0f;

			positions.push_back(position);

		} else if (identifier == "vt") {

			Vector2 texcoord;

			s >> texcoord.x >> texcoord.y;

			texcoord.y = 1.0f - texcoord.y;

			texcoords.push_back(texcoord);

		} else if (identifier == "vn") {

			Vector3 normal;

			s >> normal.x >> normal.y >> normal.z;

			normal.x *= -1.0f;

			normals.push_back(normal);

		} else if (identifier == "f") {

			VertexData triangle[3];

			// 面は三角形限定
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {

				std::string vertexDefinition;

				s >> vertexDefinition;

				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得
				std::istringstream v(vertexDefinition);

				uint32_t elementIndices[3];

				for (int32_t element = 0; element < 3; ++element) {

					std::string index;

					std::getline(v, index, '/'); // /区切りでインデックスを読む

					elementIndices[element] = std::stoi(index);

				}

				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築
				Vector4 position = positions[elementIndices[0] - 1];

				Vector2 texcoord = texcoords[elementIndices[1] - 1];

				Vector3 normal = normals[elementIndices[2] - 1];

				/*VertexData vertex = { position,texcoord,normal };

				modelData.vertices.push_back(vertex);*/

				triangle[faceVertex] = { position,texcoord,normal };

			}

			// 頂点を逆にすることで周り順を逆にする
			modelData.vertices.push_back(triangle[2]);

			modelData.vertices.push_back(triangle[1]);

			modelData.vertices.push_back(triangle[0]);

		} else if (identifier == "mtllib") {

			// materialTemplateLibraryのファイル名を取得
			std::string materialFilename;

			s >> materialFilename;

			// 基本的にobjファイルと同一階層にmtlは存在させるのでディレクトリメイトファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);

		}

	}

	// ModelDataを返す
	return modelData;

}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	CoInitializeEx(0, COINIT_MULTITHREADED);

	Microsoft::WRL::ComPtr<IDXGIDebug> debug;

	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {

		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);

		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);

		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);

		//	debug->Release();

	}

#pragma region Windowの生成


#pragma endregion

	WinApp* winApp = nullptr;
	winApp = new WinApp();
	winApp->Initialize();

	Input* input = nullptr;
	input = new Input();
	input->Initialize(winApp);
	input->Update();

	//ポインタ
	DirectXCommon* dxCommon = nullptr;

	//DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);



	
		// モデル読み込み
		ModelData modelData = LoadObjFile("resources", "plane.obj");
	
		// 頂点リソースを作る
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	
		// 頂点バッファビューを作成
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress(); // リソースの先頭のアドレスから使う
	
		vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size()); // 使用するリソースのサイズは頂点のサイズ
	
		vertexBufferView.StrideInBytes = sizeof(VertexData); // 1頂点辺りのサイズ
	
		// 頂点リソースにデータを書き込む
		VertexData* vertexData = nullptr;
	
		vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData)); // 書き込むためのアドレスを取得
	
		std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
	
		// 2枚目のTextureを読んで転送する
		DirectX::ScratchImage mipImages2 = dxCommon->LoadTexture(modelData.material.textureFilePath);
	
		const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	
		Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata2);
	
		dxCommon->UploadTextureData(textureResource2.Get(), mipImages2);
	
		// metaDataを基にSRVの作成
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	
		srvDesc2.Format = metadata2.format;
	
		srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	
		srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	
		
		srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);


		//SRVを作成するDescriptorHeapの場所を決める
		D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = dxCommon->GetSRVCPUDescriptorHandle(1);

		D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = dxCommon->GetSRVGPUDescriptorHandle(1);



		//SRVの生成
		dxCommon->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);




	
		// Textureを読んで転送する
		DirectX::ScratchImage mipImages = dxCommon->LoadTexture("resources/UVChecker.png");
	
		const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	
		Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata);
	
		dxCommon-> UploadTextureData(textureResource.Get(), mipImages);
	
		// metaDataを基にSRVの設定
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	
		srvDesc.Format = metadata.format;
	
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	
		srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	
		 //SRVを作成するDescriptorHeapの場所を決める
		D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(2);
	
		D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(2);
	
		 //先頭はimGuiが使っているのでその次を使う
		textureSrvHandleCPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
		textureSrvHandleGPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
		 //SRVの生成
		dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);
	


		D3D12_ROOT_PARAMETER rootParameters[4] = {};
	
		rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	
		rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	
		rootParameters[0].Descriptor.ShaderRegister = 0;
	
		rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	
		rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	
		rootParameters[1].Descriptor.ShaderRegister = 0;
	
		rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // Descriptortableを使う
	
		rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	
		//rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	
		//rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数
	
		rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	
		rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	
		rootParameters[3].Descriptor.ShaderRegister = 1; // レジスタ番号1を使う
	
		D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	
		staticSamplers[0].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	
		staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	
		staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	
		staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	
		staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	
		staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	
		staticSamplers[0].ShaderRegister = 0;
	
		staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	
		D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};

		descriptionRootSignature.Flags =

			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};

		descriptorRange[0].BaseShaderRegister = 0; // 0から始める

		descriptorRange[0].NumDescriptors = 1; // 数は1つ

		descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う

		descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // offsetを指導計算

		descriptionRootSignature.pStaticSamplers = staticSamplers;
	
		descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
	
		descriptionRootSignature.pParameters = rootParameters;
	
		descriptionRootSignature.NumParameters = _countof(rootParameters);
	
		// Sprite用のMaterialResourceを作る
		Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = dxCommon-> CreateBufferResource( sizeof(Material));
	
		Material* materialDataSprite = nullptr;
	
		materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	
		materialDataSprite->enableLighting = false;
	
		materialDataSprite->color = { 1.0f,1.0f,1.0f,1.0f };
	
		materialDataSprite->uvTransform = MakeIdentity4x4();
	
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = dxCommon-> CreateBufferResource(sizeof(DirectionalLight));
	
		DirectionalLight* directionalLightData = nullptr;
	
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	
		// デフォルト値
		directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	
		directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	
		directionalLightData->intensity = 1.0f;

		ID3DBlob* signatureBlob = nullptr;

		ID3DBlob* errorBlob = nullptr;

		HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,

			D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

		if (FAILED(hr)) {

			Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));

			assert(false);

		}

	
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	
		inputElementDescs[0].SemanticName = "POSITION";
	
		inputElementDescs[0].SemanticIndex = 0;
	
		inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	
		inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	
		inputElementDescs[1].SemanticName = "TEXCOORD";
	
		inputElementDescs[1].SemanticIndex = 0;
	
		inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	
		inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	
		inputElementDescs[2].SemanticName = "NORMAL";
	
		inputElementDescs[2].SemanticIndex = 0;
	
		inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	
		inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	
		D3D12_INPUT_LAYOUT_DESC inputLayOutDesc{};
	
		inputLayOutDesc.pInputElementDescs = inputElementDescs;
	
		inputLayOutDesc.NumElements = _countof(inputElementDescs);
	
		D3D12_BLEND_DESC blendDesc{};
	
		blendDesc.RenderTarget[0].RenderTargetWriteMask =
	
			D3D12_COLOR_WRITE_ENABLE_ALL;
	
		D3D12_RASTERIZER_DESC rasterizerDesc{};
	
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	
		IDxcBlob* vertexShaderBlob = dxCommon->CompileShader(L"resources/shaders/Object3D.VS.hlsl",
	
			L"vs_6_0");
	
		assert(vertexShaderBlob != nullptr);
	
		IDxcBlob* pixelShaderBlob = dxCommon-> CompileShader(L"resources/shaders/Object3D.PS.hlsl",
	
			L"ps_6_0");
	
		assert(pixelShaderBlob != nullptr);

		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

		hr = dxCommon->GetDevice()->CreateRootSignature(0,

			signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),

			IID_PPV_ARGS(&rootSignature));

		assert(SUCCEEDED(hr));
	
		// DepthStencilStateの設定
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	
		// Depthの機能を有効化
		depthStencilDesc.DepthEnable = true;
	
		// 書き込み
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	
		 //近ければ描画
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipeLineStateDesc{};
	
		graphicsPipeLineStateDesc.pRootSignature = rootSignature.Get();
	
		graphicsPipeLineStateDesc.InputLayout = inputLayOutDesc;
	
		graphicsPipeLineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	
		vertexShaderBlob->GetBufferSize() };
	
		graphicsPipeLineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	
		pixelShaderBlob->GetBufferSize() };
	
		graphicsPipeLineStateDesc.BlendState = blendDesc;
	
		graphicsPipeLineStateDesc.RasterizerState = rasterizerDesc;
	
		graphicsPipeLineStateDesc.NumRenderTargets = 1;
	
		graphicsPipeLineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	
		graphicsPipeLineStateDesc.PrimitiveTopologyType =
	
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	
		graphicsPipeLineStateDesc.SampleDesc.Count = 1;
	
		graphicsPipeLineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	
		// DepthStencilの設定
		graphicsPipeLineStateDesc.DepthStencilState = depthStencilDesc;
	
		graphicsPipeLineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
		Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	
	hr = dxCommon-> GetDevice()->CreateGraphicsPipelineState(&graphicsPipeLineStateDesc,
	
			IID_PPV_ARGS(&graphicsPipelineState));
	
		assert(SUCCEEDED(hr));
	
		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxCommon-> CreateBufferResource(sizeof(Material));
	
		Material* materialData = nullptr;
	
		materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	
		materialData->enableLighting = true;
	
		materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	
		materialData->uvTransform = MakeIdentity4x4();
	
		Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = dxCommon-> CreateBufferResource(sizeof(TransformationMatrix));
	
		TransformationMatrix* wvpData = nullptr;
	
		wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	
		wvpData->World = MakeIdentity4x4();
	
		wvpData->WVP = MakeIdentity4x4();
	
	
		Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = dxCommon-> CreateBufferResource(sizeof(uint32_t) * 6);
	
		D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	
		// リソースの先頭のアドレスから使う
		indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	
		// 使用するリソースのサイズはインデックス6つ分
		indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	
		// インデックスはuint32_t
		indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;
	
		// インデックスリソースにデータを書き込む
		uint32_t* indexDataSprite = nullptr;
	
		indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	
		indexDataSprite[0] = 0;
		indexDataSprite[1] = 1;
		indexDataSprite[2] = 2;
		indexDataSprite[3] = 1;
		indexDataSprite[4] = 3;
		indexDataSprite[5] = 2;
	
		// Sprite用の頂点リソース
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = dxCommon-> CreateBufferResource(sizeof(VertexData) * 4);
	
		//	頂点バッファビュー
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	
		// リソースの先頭のアドレスから使う
		vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	
		// 使用するリソースのサイズは頂点6つ分
		vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	
		// 1頂点辺りのサイズ
		vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);
	
		VertexData* vertexDataSprite = nullptr;
	
		vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	
		// 1枚目の三角形
		vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };
		vertexDataSprite[0].texcoord = { 0.0f,1.0f };
		vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	
		vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };
		vertexDataSprite[1].texcoord = { 0.0f,0.0f };
		vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	
		vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };
		vertexDataSprite[2].texcoord = { 1.0f,1.0f };
		vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };
	
		// 2枚目の三角形
		vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f };
		vertexDataSprite[3].texcoord = { 1.0f,0.0f };
		vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };
	
	
		// Sprite用のTransformationMatrix用のリソースを作る
		Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = dxCommon-> CreateBufferResource(sizeof(TransformationMatrix));
	
		// データを書き込む
		TransformationMatrix* transformationMatrixDataSprite = nullptr;
	
		// 書き込むためのアドレスを取得
		transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	
		// 単価行列を書き込んでおく
		transformationMatrixDataSprite->WVP = MakeIdentity4x4();
	
		transformationMatrixDataSprite->World = MakeIdentity4x4();
	
		bool useMonsterBall = true;
	
		MSG msg{};
	

	while (true) {

		//描画前処理
		dxCommon->PreDraw();


		if (winApp->ProcessMessage()) {

			break;

		}

		//
		//			input->Update();
		//
					ImGui_ImplDX12_NewFrame();
		
					ImGui_ImplWin32_NewFrame();
		
					ImGui::NewFrame();
		//
		//			if (input->TriggerKey(DIK_0)) {
		//
		//				OutputDebugStringA("Hit 0\n");
		//
		//			}
		//
					//各種行列の計算
					Matrix4x4 worldMatrix = MakeAffinMatrix(transform.scale, transform.rotate, transform.translate);
		
					Matrix4x4 cameraMatrix = MakeAffinMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		
					Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		
					Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
		
					Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		
					Matrix4x4 worldMatrixSprite = MakeAffinMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		
					Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
		
					Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
		
					Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
		
					Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
		
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
		
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
		
					materialDataSprite->uvTransform = uvTransformMatrix;
		
					transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
		
					transformationMatrixDataSprite->World = worldViewProjectionMatrixSprite;
		
					ImGui::Begin("Window");
		
					ImGui::DragFloat3("color", &materialData->color.x, 0.01f);
					ImGui::DragFloat3("translate", &transform.translate.x, 0.01f);
					ImGui::DragFloat3("scale", &transform.scale.x, 0.01f);
					ImGui::DragFloat3("rotate", &transform.rotate.x, 0.01f);
					ImGui::DragFloat3("sprite.transform", &transformSprite.translate.x, 0.3f);
					ImGui::DragFloat2("sprite.scale", &transformSprite.scale.x, 0.01f);
					ImGui::DragFloat2("sprite.rotate", &transformSprite.rotate.x, 0.01f);
					//ImGui::Checkbox("useMonsterBall", &useMonsterBall);
					ImGui::SliderAngle("Light.color", &directionalLightData->color.x, 0.01f);
					ImGui::SliderAngle("Light.direction", &directionalLightData->direction.x, 0.01f);
					ImGui::SliderAngle("Light.intensity", &directionalLightData->intensity, 0.01f);
					ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
					ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
					ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
					ImGui::End();
		
					//ImGui::ShowDemoWindow();
		//
		//			wvpData->World = worldViewProjectionMatrix;
		//
		//			wvpData->WVP = worldViewProjectionMatrix;
		//
		//			scissorRect.bottom = WinApp::kClientHeight;
		//
					ImGui::Render();
		//
		//			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);

		//			commandList->SetGraphicsRootSignature(rootSignature.Get());
		//
		//			commandList->SetPipelineState(graphicsPipelineState.Get());
		//
		//			commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		//
		//			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//
		//			commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		//
		//			commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
		//
		//			commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		//
		//			// SRVのDescriptortableの先頭を設定。2はrootParameter[2]である
		//			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		//
		//			//commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
		//
		//			//commandList->DrawInstanced(1536, 1, 0, 0);
		//
		//			commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
		//
		//			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
		//
		//			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//
		//			commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		//
		//			commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
		//
		//			commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
		//
		//			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		//
		//			// SRVのDescriptortableの先頭を設定。2はrootParameter[2]である
		//			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		//
		//			commandList->IASetIndexBuffer(&indexBufferViewSprite);
		//
		//			// 描画　ドローコール
		//			commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
		
		//描画後処理
		dxCommon->PostDraw();


		//			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
		//
		//			//transform.rotate.y += 0.02f;
		//
		//			wvpData->World = worldMatrix;
		//
		//			wvpData->WVP = worldMatrix;
		//
		//		}
		//
		//	ImGui_ImplDX12_Shutdown();
		//
		//	ImGui_ImplWin32_Shutdown();
		//
		//	ImGui::DestroyContext();
		//
		//	CloseHandle(fenceEvent);
		//
		//	winApp->Finalize();
		//
		//	delete input;
		//	delete winApp;
		//	delete dxCommon;
		//
		//	Log("Hello,DirectX!\n");

	}

	return 0;

} 