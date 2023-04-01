#pragma once

#include "../fireball.h"
#include "../vulkan.h"

#ifdef __linux__
#define VK_USE_PLATFORM_XCB_KHR
#elif _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(f, msg) if ((f) != 0) fb_error((msg));
#define WARN(f, msg) if ((f) != 0) return fb_warning((msg));

// A struct for model buffer.
typedef struct Model_t {
    uint32_t index_cnt;
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
} Model;

// A struct for camera uniform buffer.
typedef struct Camera_t {
    VkBuffer buffer;
    VkDeviceMemory buffer_memory;
} Camera;

// A struct for image texture.
typedef struct Image_t {
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
} Image;

// TODO: struct
typedef struct VulkanApp_t {
    // A struct for core objects
    struct Core_t {
        VkInstance instance;
        VkDevice device;
        VkPhysicalDeviceMemoryProperties phys_device_memory_prop;
    } core;
    // A struct for objects related to rendering
    struct Renderer_t {
        uint32_t width;
        uint32_t height;
        uint32_t images_cnt;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        VkImageView *image_views;
        VkQueue queue;
        VkCommandPool command_pool;
    } rendering;
    // A struct for objects for pipeline
    struct Pipeline_t {
        VkRenderPass render_pass;
        VkFramebuffer *framebuffers;
        VkShaderModule vert_shader;
        VkShaderModule frag_shader;
        VkSampler sampler;
        VkDescriptorSetLayout descriptor_set_layout;
        VkDescriptorPool descriptor_pool;
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;
    } pipeline;
    // A struct for 
    struct FrameData_t {
        VkCommandBuffer command_buffer;
        VkFence fence;
        VkSemaphore render_semaphore;
        VkSemaphore present_semaphore;
    } framedata;
    // A struct for 
    struct Resource_t {
        // it's the same as max_image_texture_num
        uint32_t max_descriptor_set_num;
        VkDescriptorSet *descriptor_sets;
        Camera camera;
        // it's guaranteed to be greater than 1
        uint32_t max_image_texture_num;
        Image *image_textures;
        Model square;
    } resource;
} VulkanApp;

// A function to get a memory type index that's available and bitted at flags.
// It returns -1 if it fails.
inline static int32_t get_memory_type_index(VulkanApp *app, VkMemoryRequirements reqs, VkMemoryPropertyFlags flags) {
    for (int32_t i = 0; i < app->core.phys_device_memory_prop.memoryTypeCount; ++i) {
        if ((reqs.memoryTypeBits & (1 << i)) && (app->core.phys_device_memory_prop.memoryTypes[i].propertyFlags & flags)) {
            return i;
        }
    }
    return -1;
}

// A function to create a buffer.
inline static warn_t create_buffer(
    VulkanApp *app,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags flags,
    VkBuffer *p_buffer,
    VkDeviceMemory *p_device_memory
) {
    VkBufferCreateInfo buffer_create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        NULL,
        0,
        size,
        usage,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
    };
    WARN(vkCreateBuffer(app->core.device, &buffer_create_info, NULL, p_buffer), "");
    VkMemoryRequirements reqs;
    vkGetBufferMemoryRequirements(app->core.device, *p_buffer, &reqs);
    VkMemoryAllocateInfo allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        NULL,
        reqs.size,
        0,
    };
    allocate_info.memoryTypeIndex = get_memory_type_index(app, reqs, flags);
    WARN(vkAllocateMemory(app->core.device, &allocate_info, NULL, p_device_memory), "");
    WARN(vkBindBufferMemory(app->core.device, *p_buffer, *p_device_memory, 0), "");
    return 1;
}

// A function to map data into memory.
inline static warn_t map_memory(VulkanApp *app, VkDeviceMemory device_memory, void *data, int32_t size) {
    void *p;
    WARN(vkMapMemory(app->core.device, device_memory, 0, VK_WHOLE_SIZE, 0, &p), "");
    memcpy(p, data, size);
    vkUnmapMemory(app->core.device, device_memory);
    return 1;
}

// A function to load image texture into app.resource.image_textures[id].
// WARN: it overwrites data at `id` and doesn't update descriptor set.
warn_t load_image_texture(const unsigned char *pixels, int32_t width, int32_t height, int32_t id);
