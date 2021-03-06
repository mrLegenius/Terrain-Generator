#pragma once
#include <memory>
#include <vector>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

class Shape3D
{
	std::unique_ptr<IndexBuffer> m_IndexBuffer;
	std::unique_ptr<VertexArray> m_ObjectVAO;
	std::unique_ptr<VertexBuffer> m_VertexBuffer;
public:
	IndexBuffer& getIndexBuffer()
	{
		return *m_IndexBuffer;
	}
	VertexArray& getObjectVAO()
	{
		return *m_ObjectVAO;
	}
	VertexBuffer& getVertexBuffer()
	{
		return *m_VertexBuffer;
	}

	Shape3D(std::vector<GLfloat> vertices, std::vector<GLuint> indices, VertexBufferLayout& layout)
	{
		m_ObjectVAO = std::make_unique<VertexArray>();
		m_VertexBuffer = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(float));

		m_ObjectVAO->AddBuffer(*m_VertexBuffer, layout);
		m_IndexBuffer = std::make_unique<IndexBuffer>(indices.data(), indices.size());
	}

	static std::shared_ptr<Shape3D> CreatePlane(GLuint dimensions)
	{
		std::vector<float> vertices;
		std::vector<unsigned int> indices;

		float half = (dimensions - 1) / 2.0f;

		float size = 1.f / (dimensions - 1);
		float halfSize = half * size;

		unsigned int index = 0;
		for (GLuint x = 0; x < dimensions; x++)
		{
			for (GLuint y = 0; y < dimensions; y++)
			{
				vertices.push_back(x * size - halfSize);
				vertices.push_back(y * size - halfSize);
				vertices.push_back(0);

				vertices.push_back(0);
				vertices.push_back(0);
				vertices.push_back(-1);

				vertices.push_back(x * size);
				vertices.push_back(y * size);
			}
		}

		for (GLuint row = 0; row < dimensions - 1; row++)
		{
			for (GLuint col = 0; col < dimensions - 1; col++)
			{
				indices.push_back(dimensions * row + col);
				indices.push_back(dimensions * row + col + dimensions);
				indices.push_back(dimensions * row + col + dimensions + 1);

				indices.push_back(dimensions * row + col);
				indices.push_back(dimensions * row + col + dimensions + 1);
				indices.push_back(dimensions * row + col + 1);
			}
		}

		VertexBufferLayout layout;
		layout.Push<float>(3);
		layout.Push<float>(3);
		layout.Push<float>(2);

		return std::make_shared<Shape3D>(vertices, indices, layout);
	}

	static std::shared_ptr<Shape3D> CreateSphere(const GLuint numberSlices) 
	{
		GLuint numberParallels = numberSlices / 2;

		const GLfloat radius = 1u;
		assert(numberSlices >= 3u);

		GLfloat angleStep = 2 * glm::pi<float>() / numberSlices;

		std::vector<float> vertices;
		std::vector<unsigned int> indices;

		for (GLuint i = 0u; i < numberParallels + 1u; i++) {
			for (GLuint j = 0u; j < numberSlices + 1u; j++) {
				GLuint index = (i * (numberSlices + 1u) + j);

				GLfloat x = radius * sinf(angleStep * (GLfloat)i) * sinf(angleStep * (GLfloat)j);
				GLfloat y = radius * cosf(angleStep * (GLfloat)i);
				GLfloat z = radius * sinf(angleStep * (GLfloat)i) * cosf(angleStep * (GLfloat)j);

				//position
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);

				//Normal
				vertices.push_back(x / radius);
				vertices.push_back(y / radius);
				vertices.push_back(z / radius);

				//TexCoords
				vertices.push_back((GLfloat)j / (GLfloat)numberSlices);
				vertices.push_back(((GLfloat)i / numberParallels));
			}
		}

		for (GLuint i = 0u; i < numberParallels; i++) {
			for (GLuint j = 0u; j < numberSlices; j++) {
				indices.push_back(i * (numberSlices + 1u) + j);
				indices.push_back((i + 1u) * (numberSlices + 1u) + j);
				indices.push_back((i + 1u) * (numberSlices + 1u) + (j + 1u));

				indices.push_back(i * (numberSlices + 1u) + j);
				indices.push_back((i + 1u) * (numberSlices + 1u) + (j + 1u));
				indices.push_back(i * (numberSlices + 1u) + (j + 1u));
			}
		}

		VertexBufferLayout layout;
		layout.Push<float>(3);
		layout.Push<float>(3);
		layout.Push<float>(2);

		return std::make_shared<Shape3D>(vertices, indices, layout);
	}

	static std::shared_ptr<Shape3D> CreateCone(const GLuint numberSlices) {
		const GLuint numberParallels = numberSlices;

		const GLfloat topRadius = 0;
		const GLfloat bottomRadius = 0.5f;

		std::vector<float> vertices;
		std::vector<unsigned int> indices;

		assert(numberSlices >= 3u);

		GLfloat angleStep = 2 * glm::pi<float>() / numberSlices;
		GLfloat height = 1.0f;
		GLfloat halfHeight = 0.5f;

		for (GLuint i = 0u; i <= numberParallels; i++) {
			for (GLuint j = 0u; j <= numberSlices; j++) {
				GLfloat radius = i * bottomRadius / numberParallels;

				auto x = radius * cosf(angleStep * (GLfloat)j);
				auto y = halfHeight - i * (height / numberParallels);
				auto z = radius * sinf(angleStep * (GLfloat)j);
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);

				vertices.push_back(x / radius);
				vertices.push_back(y / radius);
				vertices.push_back(z / radius);

				vertices.push_back((GLfloat)j / (GLfloat)numberSlices);
				vertices.push_back(1.0f - (GLfloat)i / (GLfloat)numberParallels);

			}
		}

		GLuint indexIndices = 0u;

		for (GLuint i = 0u; i < numberParallels; i++) {
			for (GLuint j = 0u; j < numberSlices; j++) {
				indices.push_back(i * (numberSlices + 1u) + j);
				indices.push_back((i + 1u) * (numberSlices + 1u) + (j + 1u));
				indices.push_back((i + 1u) * (numberSlices + 1u) + j);

				indices.push_back(i * (numberSlices + 1u) + j);
				indices.push_back(i * (numberSlices + 1u) + (j + 1u));
				indices.push_back((i + 1u) * (numberSlices + 1u) + (j + 1u));
			}
		}

		vertices.push_back(0.0f);
		vertices.push_back(-halfHeight);
		vertices.push_back(0.0f);

		vertices.push_back(0.0f);
		vertices.push_back(-1.0f);
		vertices.push_back(0.0f);

		vertices.push_back(1.0f);
		vertices.push_back(1.0f);

		for (GLuint i = 0u; i < numberSlices; i++) {
			indices.push_back(numberParallels * (numberSlices + 1u) + (i + 1u));
			indices.push_back(vertices.size() / 8 - 1u);
			indices.push_back(numberParallels * (numberSlices + 1u) + i);
		}

		VertexBufferLayout layout;
		layout.Push<float>(3);
		layout.Push<float>(3);
		layout.Push<float>(2);

		return std::make_shared<Shape3D>(vertices, indices, layout);
	}

	static std::shared_ptr<Shape3D> CreateCube()
	{
		float height = 0.5f;

		std::vector<GLfloat> vertices = {
		 //positions 		  normals			   texCoords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		};

		std::vector<GLuint> indices = { 
			0, 2, 1,		0, 3, 2, 
			4, 5, 6,		4, 6, 7, 
			8, 9, 10,		8, 10, 11, 
			12, 15, 14,		12, 14, 13, 
			16, 17, 18,		16, 18, 19, 
			20, 23, 22,		20, 22, 21 };

		VertexBufferLayout layout;
		layout.Push<float>(3);
		layout.Push<float>(3);
		layout.Push<float>(2);

		return std::make_shared<Shape3D>(vertices, indices, layout);
	}
};

