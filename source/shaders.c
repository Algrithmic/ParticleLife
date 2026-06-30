#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include "glad/glad.h"

#include "shaders.h"
#include "particle.h"

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

/**
 * shader_compilation_status
 *
 * @brief Checks whether a shader object compiled successfully and logs any errors.
 *
 * Queries the GL_COMPILE_STATUS of the given shader and prints the info log
 * to stdout if compilation failed.
 *
 * @param shader    The OpenGL shader object ID to check.
 * @param filename  The source filename, used in the error message for identification.
 * @return          Non-zero if compilation succeeded, 0 on failure.
 *
 * @note This is a static internal helper and should only be called from compile_shader().
 */
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

/**
 * compile_shader
 *
 * @brief Reads a GLSL shader source file from disk and compiles it.
 *
 * Opens the file at the given path, reads its full contents into a buffer,
 * creates an OpenGL shader object of the specified type, and compiles it.
 *
 * @param filename  Path to the GLSL shader source file.
 * @param type      OpenGL shader type (e.g. GL_VERTEX_SHADER, GL_FRAGMENT_SHADER,
 *                  GL_COMPUTE_SHADER).
 * @return          The compiled OpenGL shader object ID on success, or 0 on failure.
 *
 * @note This is a static internal helper called by init_graphics() and init_compute().
 * @warning The caller is responsible for calling glDeleteShader() on the returned
 *          shader ID once it has been attached to a program.
 */
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
    if (contents == NULL) 
        return 0;

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

/**
 * program_compilation_status
 *
 * @brief Checks whether a shader program linked successfully and logs any errors.
 *
 * Queries the GL_LINK_STATUS of the given program and prints the info log
 * to stdout if linking failed.
 *
 * @param program  The OpenGL shader program ID to check.
 * @return         Non-zero if linking succeeded, 0 on failure.
 *
 * @note This is a static internal helper called by init_graphics() and init_compute().
 */
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

/**
 * init_vertices
 *
 * @brief Allocates and configures the VAO and VBOs for particle rendering.
 *
 * Creates a VAO, a static VBO for the shared particle shape vertices, and a
 * dynamic VBO for per-instance particle data. Configures vertex attribute
 * pointers for position (location 0), instance position (location 1), and
 * instance class (location 2). Stores the resulting VAO and instance VBO
 * handles in shader_data.
 *
 * @param shader_data  Pointer to the shader state; vao and vbo fields are set on return.
 * @param particles    Pointer to the particle array used to populate the instance VBO.
 *
 * @note This is a static internal helper and should only be called from init_graphics().
 */
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

/**
 * has_extension
 *
 * @brief Checks whether a filename ends with the given extension.
 *
 * @param filename   The filename or path to check.
 * @param extension  The expected extension, including the dot (e.g. ".vert").
 * @return           1 if the filename ends with the extension, 0 otherwise.
 *
 * @note This is a static internal helper used by init_graphics() to dispatch
 *       shader files to the correct compile call by file extension.
 */
static uint8_t has_extension(char const *filename, char const *extension) {
    char const *dot = strrchr(filename, '.');
    if (dot == NULL)
        return 0;
    return strcmp(dot, extension) == 0;
}

/**
 * init_graphics
 *
 * @brief Compiles and links the graphics shader program from a variadic list of shader files.
 *
 * Accepts a variable number of shader source filenames, dispatching each to
 * compile_shader() based on file extension (.vert or .frag). Links the compiled
 * shaders into a graphics program and initializes vertex buffers via init_vertices().
 *
 * @param shader_data  Pointer to the shader state; graphics_program, vao, and vbo
 *                     are set on success.
 * @param particles    Pointer to the particle array passed to init_vertices().
 * @param count        Number of shader filenames in the variadic argument list.
 * @param ...          Shader source filenames (char *); must match count.
 * @return             1 on success, 0 if any shader fails to compile or link.
 *
 * @warning Files with unrecognized extensions are skipped with a warning, which
 *          may cause program linking to fail if a required shader stage is missing.
 * @see init_compute(), init_vertices()
 */
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
        glDeleteProgram(shader_data->graphics_program);
        return 0;
    }

    init_vertices(shader_data, particles);
    return 1;
}

/**
 * init_compute
 *
 * @brief Compiles and links a compute shader program from a single source file.
 *
 * Compiles the given .comp shader file and links it into a standalone compute
 * program stored in shader_data->compute_program.
 *
 * @param shader_data  Pointer to the shader state; compute_program is set on success.
 * @param filename     Path to the compute shader source file (.comp).
 * @return             1 on success, 0 if the shader fails to compile or link.
 *
 * @see init_graphics()
 */
uint8_t init_compute(shader_t *shader_data, char const *filename) {
    shader_data->compute_program = glCreateProgram();
    uint32_t compute_shader = compile_shader(filename, GL_COMPUTE_SHADER);

    if (!compute_shader) {
        (void) printf("ERROR : SHADER PROGRAM - Failed to compile one or more shaders\n");
        glDeleteShader(compute_shader);
        glDeleteProgram(shader_data->compute_program);
        return 0;
    }

    glAttachShader(shader_data->compute_program, compute_shader);
    glLinkProgram(shader_data->compute_program);
    glDeleteShader(compute_shader);

    if (!program_compilation_status(shader_data->compute_program)) {
        glDeleteShader(compute_shader);
        glDeleteProgram(shader_data->compute_program);
        return 0;
    }

    return 1;
}