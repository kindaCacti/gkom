#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <iostream>
#include <map>
#include <string>

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
    unsigned int TextureID; 
    glm::ivec2   Size;      
    glm::ivec2   Bearing;   
    unsigned int Advance;   
};

class TextRenderer {
public:
    std::map<char, Character> Characters;
    unsigned int VAO, VBO;
    unsigned int ShaderProgram;

    TextRenderer() : VAO(0), VBO(0), ShaderProgram(0) {}
    ~TextRenderer() {
        CleanUp();
    }

    bool Init(const char* fontPath, unsigned int fontSize, int windowWidth, int windowHeight) {
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return false;
        }

        FT_Face face;
        if (FT_New_Face(ft, fontPath, 0, &face)) {
            std::cerr << "ERROR::FREETYPE: Failed to load font: " << fontPath << std::endl;
            FT_Done_FreeType(ft);
            return false;
        }

        FT_Set_Pixel_Sizes(face, 0, fontSize);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (unsigned char c = 32; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cerr << "ERROR::FREETYPE: Failed to load Glyph: " << c << std::endl;
                continue;
            }

            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED, 
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        CompileShaders();
        SetupBuffers();
        UpdateProjection(windowWidth, windowHeight);

        return true;
    }

    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color) {
        glUseProgram(ShaderProgram);
        glUniform3f(glGetUniformLocation(ShaderProgram, "textColor"), color.x, color.y, color.z);
        
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        for (std::string::const_iterator c = text.begin(); c != text.end(); c++) {
            // SAFE LOOKUP FIX: Prevents dirty map writes and crashes
            auto it = Characters.find(*c);
            if (it == Characters.end()) {
                continue; 
            }
            Character ch = it->second;

            float xpos = x + ch.Bearing.x * scale;
            float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

            float w = ch.Size.x * scale;
            float h = ch.Size.y * scale;

            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },            
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }           
            };

            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
            
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            x += (ch.Advance >> 6) * scale; 
        }
        
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void UpdateProjection(int width, int height) {
        glUseProgram(ShaderProgram);
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    }

    void CleanUp() {
        for (auto const& [key, val] : Characters) {
            glDeleteTextures(1, &val.TextureID);
        }
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
        glDeleteProgram(ShaderProgram);
    }

private:
    void SetupBuffers() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void CompileShaders() {
        const char* vertexShaderSource = R"glsl(
            #version 330 core
            layout (location = 0) in vec4 vertex; 
            out vec2 TexCoords;
            uniform mat4 projection;
            void main() {
                gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
                TexCoords = vertex.zw;
            }
        )glsl";

        const char* fragmentShaderSource = R"glsl(
            #version 330 core
            in vec2 TexCoords;
            out vec4 FragColor;

            uniform sampler2D textTexture;
            uniform vec3 textColor;

            void main() {    
                float alphaValue = texture(textTexture, TexCoords).r;
                FragColor = vec4(textColor, alphaValue);
            }
        )glsl";

        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        ShaderProgram = glCreateProgram();
        glAttachShader(ShaderProgram, vertexShader);
        glAttachShader(ShaderProgram, fragmentShader);
        glLinkProgram(ShaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // UNIFORM BINDING FIX: Connect texture unit 0 explicitly
        glUseProgram(ShaderProgram);
        glUniform1i(glGetUniformLocation(ShaderProgram, "textTexture"), 0);
    }
};

#endif