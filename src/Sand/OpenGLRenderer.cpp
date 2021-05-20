#include "Core.h"
#include "Game.h"
#include "../Vendor/glad/gl.h"

//If return value is 0, an error occured
internal u32 OpenGLPrepShaders(char* VertexPath, char* FragmentPath)
{
	u32 ShaderProgram = 0;
	DebugPlatformReadFileResult VertexFile = DebugPlatformReadEntireFile(VertexPath);
	DebugPlatformReadFileResult FragmentFile = DebugPlatformReadEntireFile(FragmentPath);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	// Send the vertex shader source code to GL
	const GLchar *source = (const GLchar *)VertexFile.Content;
	glShaderSource(vertexShader, 1, &source, 0);

	// Compile the vertex shader
	glCompileShader(vertexShader);

	GLint isCompiled = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		GLchar* infoLog = new GLchar[maxLength];
		//std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

		OutputDebugStringA(infoLog);
		// We don't need the shader anymore.
		glDeleteShader(vertexShader);

		// Use the infoLog as you see fit.
		return ShaderProgram;
	}

	// Create an empty fragment shader handle
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Send the fragment shader source code to GL
	// Note that std::string's .c_str is NULL character terminated.
	source = (const GLchar *)FragmentFile.Content;
	glShaderSource(fragmentShader, 1, &source, 0);

	// Compile the fragment shader
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		//GLint maxLength = 0;
		//glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		//GLchar infoLog(2048);
		//glGetShaderInfoLog(fragmentShader, 2048, 2048, &infoLog[0]);
		//OutputDebugStringA(infoLog);
		// We don't need the shader anymore.
		glDeleteShader(fragmentShader);

		// Use the infoLog as you see fit.

		// In this simple program, we'll just leave
		return ShaderProgram;
	}

	// Vertex and fragment shaders are successfully compiled.
	// Now time to link them together into a program.
	// Get a program object.
	GLuint program = glCreateProgram();

	// Attach our shaders to our program
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	// Link our program
	glLinkProgram(program);
	ShaderProgram = program;
	// Note the different functions here: glGetProgram* instead of glGetShader*.
	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE)
	{
		//GLint maxLength = 0;
		//glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		//std::vector<GLchar> infoLog(2048);
		//glGetProgramInfoLog(program, 2048, 2048, &infoLog[0]);

		// We don't need the program anymore.
		glDeleteProgram(program);
		// Don't leak shaders either.
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		// Use the infoLog as you see fit.

		// In this simple program, we'll just leave
		ShaderProgram = 0;
		return ShaderProgram;
	}

	// Always detach shaders after a successful link.
	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);

	return ShaderProgram;
}

//For now, the vertex buffer will just have the size of the triangle vertices 
//TODO: Implement batch rendering
internal OpenGLRendererState OpenGLRendererInit()
{
	OpenGLRendererState RendererState= {};

	glGenVertexArrays(1, &RendererState.VertexArrayHandle);
	glBindVertexArray(RendererState.VertexArrayHandle);

    u32 VertexBufferHandle;
    glGenBuffers(1, &VertexBufferHandle);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferHandle);

    float Vertices[] = {-0.5f, -0.5f, 0.0f,
                          0.5f,  -0.5f, 0.0f,
                          0.0f,  0.5f, 0.0f};

	RendererState.ShaderProgram = OpenGLPrepShaders("shaders/BasicPlot.vs", "shaders/BasicFill.fs");

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	//Specify vertex format
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);
	glEnableVertexAttribArray(0);

	//glViewport(0, 0, 1280, 720);

	return RendererState;
}