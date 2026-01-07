#pragma once

#include <glad/glad.h>
#include <string>

class ShaderManager {
private:
    GLuint shaderProgram;
    
    GLuint compileShader(const std::string& source, GLenum type);
    std::string loadShaderFromFile(const std::string& filepath);

public:
    ShaderManager();
    ~ShaderManager();
    
    bool loadShadersFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    bool loadShadersFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    
    void use() const;
    GLuint getProgram() const { return shaderProgram; }
    
    // Uniform setters
    void setFloat(const std::string& name, float value) const;
    void setInt(const std::string& name, int value) const;
    void setBool(const std::string& name, bool value) const;
    
    void cleanup();
};