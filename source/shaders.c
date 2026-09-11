/**
 * @file shaders.c
 * @brief GLSL shader loading, compilation, linking, and vertex buffer setup.
 *
 * Provides helpers to read shader sources from disk, compile and link graphics
 * and compute programs, report compile/link errors, and configure the VAO/VBOs
 * used for instanced particle rendering.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include "glad/glad.h"

#include "application.h"
#include "shaders.h"
#include "particle.h"

// vertices of a particle
static float particle_vertices[TOTAL_POINTS] = {
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
static int32_t shader_compilation_status(uint32_t const shader, char const *filename) {
    int32_t success;
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
static int32_t program_compilation_status(uint32_t program) {
    int32_t success;
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
 * pointers for the shape vertex position (location 0) and the per-instance
 * particle position (location 1, advanced once per instance). Stores the
 * resulting VAO and instance VBO handles in application->shaders.
 *
 * @param application  Pointer to the application; its particle array seeds the
 *                     instance VBO and its shaders.vao/vbo are set on return.
 *
 * @note This is a static internal helper and should only be called from init_graphics().
 */
static void init_vertices(shader_t *shaders, world_t *world) {
    uint32_t vertex_VBO;
    uint32_t instance_VBO;
    uint32_t VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &vertex_VBO);
    glGenBuffers(1, &instance_VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_vertices), particle_vertices, GL_STATIC_DRAW); 
    
    // position attribute (location = 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    
    // Bind to instance VBP
    glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);
    glBufferData(GL_ARRAY_BUFFER, world->settings.particle_count * sizeof(particle_t), world->particles, GL_DYNAMIC_DRAW);

    // Position attribute (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(particle_t), (void *) offsetof(particle_t, position));
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);  

    shaders->vao = VAO;
    shaders->vbo = instance_VBO;
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
static bool has_extension(char const *filename, char const *extension) {
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
 * @param application  Pointer to the application; its shaders.graphics,
 *                     vao, and vbo are set on success.
 * @param count        Number of shader filenames in the variadic argument list.
 * @param ...          Shader source filenames (char *); must match count.
 * @return             1 on success, 0 if any shader fails to compile or link.
 *
 * @warning Files with unrecognized extensions are skipped with a warning, which
 *          may cause program linking to fail if a required shader stage is missing.
 * @see init_compute(), init_vertices()
 */
bool init_graphics(shader_t *shaders, world_t *world, uint8_t count, ...) {
    va_list args;
    va_start(args, count);  // initialization of argument list

    shaders->graphics = glCreateProgram();
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
        glDeleteProgram(shaders->graphics);
        return false;
    }

    // Link Shaders
    glAttachShader(shaders->graphics, vertex_shader);
    glAttachShader(shaders->graphics, fragment_shader);
    glLinkProgram(shaders->graphics);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    if (!program_compilation_status(shaders->graphics)) {
        glDeleteProgram(shaders->graphics);
        return false;
    }

    init_vertices(shaders, world);
    return true;
}

/**
 * init_compute
 *
 * @brief Compiles and links a compute shader program from a single source file.
 *
 * Compiles the given .comp shader file and links it into a standalone compute
 * program stored in shaders->compute_program.
 *
 * @param shaders      Pointer to the shader state; compute_program is set on success.
 * @param filename     Path to the compute shader source file (.comp).
 * @return             1 on success, 0 if the shader fails to compile or link.
 *
 * @see init_graphics()
 */
bool init_compute(shader_t *shaders, char const *filename) {
    shaders->compute = glCreateProgram();
    uint32_t compute_shader = compile_shader(filename, GL_COMPUTE_SHADER);

    if (!compute_shader) {
        (void) printf("ERROR : SHADER PROGRAM - Failed to compile one or more shaders\n");
        glDeleteShader(compute_shader);
        glDeleteProgram(shaders->compute);
        return false;
    }

    glAttachShader(shaders->compute, compute_shader);
    glLinkProgram(shaders->compute);
    glDeleteShader(compute_shader);

    if (!program_compilation_status(shaders->compute)) {
        glDeleteShader(compute_shader);
        glDeleteProgram(shaders->compute);
        return false;
    }

    return true;
}

/**
 * init_buffers
 *
 * @brief Creates and binds the shader storage buffers shared with the compute shader.
 *
 * Allocates the particle SSBO at MAX_PARTICLES capacity and uploads the initial
 * active particles, binding it to binding point 0. Re-points the render VAO's
 * instance attribute (location 1) at this SSBO so the compute shader's output is
 * drawn directly without a copy. Then creates the attraction-matrix SSBO, uploads
 * its contents, and binds it to binding point 1.
 *
 * @param application  Pointer to the application holding the particle and
 *                     attraction data; the SSBO handles are stored in shaders.
 * @return             1 on success.
 *
 * @note Must be called after init_graphics() (which creates the VAO) and
 *       init_particles() (which fills the particle and attraction data).
 * @see  update_particle_ssbo(), update_attraction_ssbo()
 */
bool init_buffers(shader_t *shaders, world_t *world) {
    // Particles SSBO
    glGenBuffers(1, &shaders->particle_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaders->particle_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_PARTICLES * sizeof(particle_t), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, world->settings.particle_count * sizeof(particle_t), world->particles);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaders->particle_ssbo);   // binding = 0

    glBindVertexArray(shaders->vao);
    glBindBuffer(GL_ARRAY_BUFFER, shaders->particle_ssbo);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(particle_t), (void *) offsetof(particle_t, position));
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);

    // Attraction Matrix SSBO
    glGenBuffers(1, &shaders->attraction_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaders->attraction_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, world->attraction.length * sizeof(float), world->attraction.matrix, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaders->attraction_ssbo); // binding = 1

    return true;
}

/**
 * update_particle_ssbo
 *
 * @brief Re-uploads a contiguous range of particles to the particle SSBO.
 *
 * Uploads count particles starting at offset from the CPU-side particle array
 * into the corresponding region of the GPU buffer. Used to push only the changed
 * slice (e.g. newly spawned particles) rather than the whole array.
 *
 * @param application  Pointer to the application holding the particle data and SSBO.
 * @param offset       Index of the first particle to upload.
 * @param count        Number of particles to upload starting at offset.
 *
 * @see  recount_particles(), shuffle_particles()
 */
void update_particle_ssbo(shader_t *shaders, world_t *world, uint32_t offset, uint32_t count) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaders->particle_ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset * sizeof(particle_t), count * sizeof(particle_t), &world->particles[offset]);
}

/**
 * update_attraction_ssbo
 *
 * @brief Re-uploads the full attraction matrix to its SSBO.
 *
 * Copies the entire CPU-side attraction matrix into the GPU buffer so the compute
 * shader sees the latest inter-class weights. Called whenever the matrix is edited,
 * randomized, or loaded from a preset.
 *
 * @param application  Pointer to the application holding the attraction matrix and SSBO.
 *
 * @see  update_particle_ssbo()
 */
void update_attraction_ssbo(shader_t *shaders, world_t *world) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaders->attraction_ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, world->attraction.length * sizeof(float), world->attraction.matrix);
}

/**
 * init_border
 *
 * @brief Compiles the border shader program and sets up its VAO/VBO.
 *
 * Builds a minimal program (border.vert/border.frag) that draws 4 world-space
 * points as a line loop, independent of the instanced particle pipeline.
 * The VBO is allocated but left uninitialized; call update_border_vbo() to
 * populate it with the current world rectangle.
 *
 * @param shaders  Pointer to the shader state; border_program/vao/vbo are set.
 * @return         1 on success, 0 if the shaders fail to compile or link.
 * @see  update_border_vbo(), draw_border()
 */
bool init_border(shader_t *shaders) {
    uint32_t vertex_shader = compile_shader("./shaders/border.vert", GL_VERTEX_SHADER);
    uint32_t fragment_shader = compile_shader("./shaders/border.frag", GL_FRAGMENT_SHADER);

    shaders->border_program = glCreateProgram();

    if (!vertex_shader || !fragment_shader) {
        (void) printf("ERROR : SHADER PROGRAM - Failed to compile one or more border shaders\n");
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(shaders->border_program);
        return false;
    }

    glAttachShader(shaders->border_program, vertex_shader);
    glAttachShader(shaders->border_program, fragment_shader);
    glLinkProgram(shaders->border_program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    if (!program_compilation_status(shaders->border_program)) {
        glDeleteProgram(shaders->border_program);
        return false;
    }

    glGenVertexArrays(1, &shaders->border_vao);
    glGenBuffers(1, &shaders->border_vbo);

    glBindVertexArray(shaders->border_vao);
    glBindBuffer(GL_ARRAY_BUFFER, shaders->border_vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return true;
}

/**
 * update_border_vbo
 *
 * @brief Re-uploads the 4 corner points of the world-boundary rectangle.
 *
 * @param shaders       Pointer to the shader state holding the border VBO.
 * @param world_width   Current world width, in world units.
 * @param world_height  Current world height, in world units.
 *
 * @see  init_border(), draw_border()
 */
void update_border_vbo(shader_t *shaders, float world_width, float world_height) {
    float const corners[8] = {
        0.0f,        0.0f,
        world_width, 0.0f,
        world_width, world_height,
        0.0f,        world_height,
    };

    glBindBuffer(GL_ARRAY_BUFFER, shaders->border_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(corners), corners);
}

/**
 * draw_border
 *
 * @brief Draws the world-boundary outline as a white rectangle.
 *
 * @param shaders     Pointer to the shader state holding the border program/VAO.
 * @param projection  Column-major 4x4 world-to-clip-space projection matrix.
 *
 * @see  init_border(), update_border_vbo()
 */
void draw_border(shader_t *shaders, float const projection[16]) {
    glUseProgram(shaders->border_program);
    glUniformMatrix4fv(glGetUniformLocation(shaders->border_program, "projection"), 1, GL_FALSE, projection);

    glBindVertexArray(shaders->border_vao);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glBindVertexArray(0);
}