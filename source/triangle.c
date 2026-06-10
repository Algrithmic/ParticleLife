#include <stdio.h>
#include <stdlib.h>

#include "glad/glad.h"

#include "../include/triangle.h"

static float vertices[] = {
     0.5f,  0.5f, 0.0f,  // top right
     0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f   // top left 
};
static uint32_t indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};  


static int shader_compilation_status(uint32_t const shader, char const *filename) {
    int success;
    char info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        (void) printf("ERROR : SHADER %s - Compilation Failure\n%s\n", filename, info_log);
        return success;
    }
    return success;
}

static uint32_t compile_shader(char const *filename, uint32_t const type) {
    // open file
    FILE *shader_file;
    if ((shader_file = fopen(filename, "rb")) == NULL) {
        printf("File `%s` does not exist.\n", filename);
        return 0;
    }

    // get length of file
    fseek(shader_file, 0, SEEK_END); // Seek to end of the file
    uint64_t const size = ftell(shader_file);  // Get current position (= file size)
    char *contents = (char *) malloc((size + 1) * sizeof(char));

    fseek(shader_file, 0, SEEK_SET); // Rewind to beginning of the file
    if (fread(contents, 1, size, shader_file) != size) {
        free(contents);
        fclose(shader_file);
        return 0;
    }
    contents[size] = '\0';
    fclose(shader_file);

    uint32_t const shader = glCreateShader(type);
    glShaderSource(shader, 1, (char const * const *) &contents, NULL);
    glCompileShader(shader);

    free(contents);
    if (!shader_compilation_status(shader, filename)) {
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static int program_compilation_status(uint32_t program) {
    int success;
    char info_log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        (void) printf("ERROR : SHADER PROGRAM - Linking Failure\n%s\n", info_log);
        return success;
    }
    return success;
}

uint32_t link_program() {
    uint32_t shader_program = glCreateProgram();
    uint32_t vertex_shader = compile_shader("./shaders/vertex.glsl", GL_VERTEX_SHADER);
    uint32_t fragment_shader = compile_shader("./shaders/fragment.glsl", GL_FRAGMENT_SHADER);

    // Link Shaders
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    if (!program_compilation_status(shader_program)) {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(shader_program);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

uint32_t init_shaders() {
    uint32_t VBO;
    uint32_t VAO;
    uint32_t EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);    

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    return VAO;
}