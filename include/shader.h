#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
private:
    static const unsigned short infoLogSize = 512;

    std::string readShaderFile(std::string path) {
        std::string shaderCode;
        std::ifstream shaderFile;

        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {
            shaderFile.open(path);
            std::stringstream shaderStream;

            shaderStream << shaderFile.rdbuf();
            shaderCode = shaderStream.str();

            shaderFile.close();
        }
        catch (const std::ifstream::failure& e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
            std::cout << "code: " << e.code() << "\nwhat: " << e.what() << std::endl;
        }

        return shaderCode;
    }

    void compileShader(unsigned int shader, const char* shaderCode) {
        int success;
        char infoLog[infoLogSize];

        glShaderSource(shader, 1, &shaderCode, NULL);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(shader, infoLogSize, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    void linkProgram(unsigned int vertex, unsigned int fragment) {
        int success;
        char infoLog[infoLogSize];

        glAttachShader(id, vertex);
        glAttachShader(id, fragment);
        glLinkProgram(id);
        glGetProgramiv(id, GL_LINK_STATUS, &success);

        if (!success) {
            glGetProgramInfoLog(id, infoLogSize, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
    }

public:
    unsigned int id;

    Shader(std::string vertexPath, std::string fragmentPath) {
        std::cout << "--- creating new shader ---" << "\nvertex path: " << vertexPath << "\nfragmentPath: " << fragmentPath << std::endl;
        std::string vertexShaderCode = readShaderFile(vertexPath);
        std::string fragmentShaderCode = readShaderFile(fragmentPath);

        std::cout << "--- shaders read ---\n" << "vertex:\n" << vertexShaderCode << "\nfragment:\n" << fragmentShaderCode << std::endl;

        unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
        unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);

        std::cout << "compiling shaders" << std::endl;

        compileShader(vertex, vertexShaderCode.c_str());
        compileShader(fragment, fragmentShaderCode.c_str());

        std::cout << "creating shader program" << std::endl;

        id = glCreateProgram();

        linkProgram(vertex, fragment);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void use() {
        glUseProgram(id);
    }

    void setBool(const std::string& name, bool value) const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
    }

    void setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    }

    void setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(id, name.c_str()), value);
    }
};

#endif