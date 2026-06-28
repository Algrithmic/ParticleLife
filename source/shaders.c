#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include "glad/glad.h"

#include "../include/shaders.h"
#include "../include/particle.h"

// vertices of a particle
static float vertices[TOTAL_POINTS] = {
     0.000f,  0.000f,
     1.000f,  0.000f,
     0.623f,  0.782f,
    -0.223f,  0.975f,
    -0.901f,  0.434f,
    -0.901f, -0.434f,
    -0.223f, -0.975f,
     0.623f, -0.782f,
     1.000f,  0.000f,
};

// shader_compilation_status: Checks if shader file (.glsl) has been compiled successfully
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

// compile_shader: Compiles shader file "filename"
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

// program_compilation_status: Checks if shader program has been compiled successfully
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

// init_vertices: Initializes all vertices
static void init_vertices(shader_t *shader_data, particle_t *particles) {
    uint32_t vertex_VBO;
    uint32_t instance_VBO;
    uint32_t VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &vertex_VBO);
    glGenBuffers(1, &instance_VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 
    
    // position attribute (location = 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    
    // Bind to instance VBP
    glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);
    glBufferData(GL_ARRAY_BUFFER, NUM_PARTICLES * sizeof(particle_t), particles, GL_DYNAMIC_DRAW);

    // Position attribute (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(particle_t), (void *) offsetof(particle_t, position));
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    // Class attribute (location = 2)
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(particle_t), (void *) offsetof(particle_t, class));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);    

    shader_data->vao = VAO;
    shader_data->vbo = instance_VBO;
}

static uint8_t has_extension(char const *filename, char const *extension) {
    char const *dot = strrchr(filename, '.');
    if (dot == NULL)
        return 0;
    return strcmp(dot, extension) == 0;
}

uint8_t init_graphics(shader_t *shader_data, particle_t *particles, uint8_t count, ...) {
    va_list args;
    va_start(args, count);  // initialization of argument list

    shader_data->graphics_program = glCreateProgram();
    uint32_t vertex_shader = 0;
    uint32_t fragment_shader = 0;

    for (uint8_t i = 0; i < count; i++) {
        char *filename = va_arg(args, char *);
        if (has_extension(filename, ".vert"))
            vertex_shader = compile_shader(filename, GL_VERTEX_SHADER);
        else if (has_extension(filename, ".frag"))
            fragment_shader = compile_shader(filename, GL_FRAGMENT_SHADER);
        else 
            (void) printf("WARNING: Unrecognized shader extension for file '%s'", filename);
    }

    va_end(args);

    if (!vertex_shader || !fragment_shader) {
        (void) printf("ERROR : SHADER PROGRAM - Failed to compile one or more shaders\n");
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(shader_data->graphics_program);
        return 0;
    }

    // Link Shaders
    glAttachShader(shader_data->graphics_program, vertex_shader);
    glAttachShader(shader_data->graphics_program, fragment_shader);
    glLinkProgram(shader_data->graphics_program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    if (!program_compilation_status(shader_data->graphics_program)) {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(shader_data->graphics_program);
        return 0;
    }

    init_vertices(shader_data, particles);
    return 1;
}

uint8_t init_compute(shader_t *shader_data, char const *filename) {
    shader_data->compute_program = glCreateProgram();
    uint32_t compute_shader = compile_shader(filename, GL_COMPUTE_SHADER);

    if (!compute_shader) {
        (void) printf("ERROR : SHADER PROGRAM - Failed to compile one or more shaders\n");
        glDeleteShader(compute_shader);
        glDeleteProgram(compute_shader);
        return 0;
    }

    glAttachShader(shader_data->compute_program, compute_shader);
    glLinkProgram(shader_data->compute_program);
    glDeleteShader(compute_shader);

    if (!program_compilation_status(shader_data->compute_program)) {
        glDeleteShader(compute_shader);
        glDeleteProgram(compute_shader);
        return 0;
    }

    return 1;
}