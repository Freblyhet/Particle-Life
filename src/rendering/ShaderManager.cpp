#include "rendering/ShaderManager.h"
#include <iostream>
#include <fstream>
#include <sstream>

ShaderManager::ShaderManager() : shaderProgram(0) {
}

ShaderManager::~ShaderManager() {
    cleanup();
}

GLuint ShaderManager::compileShader(const std::string& source, GLenum type) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed (" 
                  << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") 
                  << "):\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

std::string ShaderManager::loadShaderFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool ShaderManager::loadShadersFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = loadShaderFromFile(vertexPath);
    std::string fragmentSource = loadShaderFromFile(fragmentPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        return false;
    }
    
    return loadShadersFromSource(vertexSource, fragmentSource);
}

bool ShaderManager::loadShadersFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    cleanup(); // Clean up any existing shader
    
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) return false;
    
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return false;
    }
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
        return false;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return true;
}

void ShaderManager::use() const {
    if (shaderProgram != 0) {
        glUseProgram(shaderProgram);
    }
}

void ShaderManager::setFloat(const std::string& name, float value) const {
    if (shaderProgram != 0) {
        glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), value);
    }
}

void ShaderManager::setInt(const std::string& name, int value) const {
    if (shaderProgram != 0) {
        glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), value);
    }
}

void ShaderManager::setBool(const std::string& name, bool value) const {
    if (shaderProgram != 0) {
        glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), value ? 1 : 0);
    }
}

void ShaderManager::cleanup() {
    if (shaderProgram != 0) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
}