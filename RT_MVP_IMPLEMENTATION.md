# RT MVP Implementation Summary

## Overview
Implemented minimal viable product (MVP) for Vulkan ray tracing path to produce visible output and align descriptor layouts with shaders.

## Changes Made

### 1. Shader Simplification (Comment 2 - MVP A)

**Modified Shaders:**
- `shaders/rt/raygen.rgen` - Removed TLAS reference, generates sky gradient directly using camera matrices
- `shaders/rt/miss.rmiss` - Updated to use `set=0,binding=1` for camera UBO
- `shaders/rt/closesthit.rchit` - Removed texture atlas and buffer references, simplified to placeholder color

**Key Changes:**
- All RT shaders now use single descriptor set (`set=0`) with:
  - `binding=0`: Storage image (output)
  - `binding=1`: Camera UBO
- Removed `traceRayEXT()` call in raygen shader (MVP - no TLAS yet)
- Raygen shader generates sky gradient based on ray direction
- Descriptor layout now matches across all RT shader stages

### 2. RTRenderer Descriptor Layout Updates

**File:** `src/vulkan/RTRenderer.cpp`, `include/poorcraft/vulkan/RTRenderer.h`

**Changes:**
- Updated `createDescriptorSets()` to use single descriptor set layout (`set=0`)
- Added `VK_IMAGE_USAGE_SAMPLED_BIT` to storage image for compositing
- Increased descriptor pool `maxSets` to 2 (RT + compositing)
- Added `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER` to pool for compositing

### 3. Compositing Pipeline (Comment 1 - Step 3)

**New Resources in RTRenderer:**
- `m_Sampler` - Linear sampler for RT output
- `m_CompositingDescriptorSetLayout` - Layout for fullscreen quad
- `m_CompositingDescriptorSet` - Descriptor set binding RT output
- `m_CompositingPipelineLayout` - Pipeline layout for compositing
- `m_CompositingPipeline` - Graphics pipeline for fullscreen quad

**New Methods:**
- `createCompositingResources()` - Creates sampler and descriptor resources
- `composite()` - Transitions RT output, begins render pass, draws fullscreen quad

**Fullscreen Shader Updates:**
- `shaders/basic/fullscreen.frag` - Updated to sample input texture instead of generating gradient

### 4. VulkanBackend Integration (Comment 1 - Step 3)

**File:** `src/vulkan/VulkanBackend.cpp`

**Changes:**
- Updated `renderUI()` for RT mode to call `RTRenderer::composite()`
- Compositing transitions storage image from `GENERAL` to `SHADER_READ_ONLY_OPTIMAL`
- Begins render pass, draws fullscreen quad sampling RT output
- ImGui renders in same pass after compositing
- Proper image layout transitions for RT write → sampling → presentation

**Added to VulkanRasterRenderer:**
- `getFramebuffer()` method to access framebuffers by image index

## Descriptor Layout Summary

### RT Pipeline
- **Set 0:**
  - Binding 0: Storage image (`VK_DESCRIPTOR_TYPE_STORAGE_IMAGE`)
  - Binding 1: Camera UBO (`VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER`)

### Compositing Pipeline
- **Set 0:**
  - Binding 0: RT output (`VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER`)

## Image Layout Transitions

1. **RT Execution:** Storage image in `VK_IMAGE_LAYOUT_GENERAL`
2. **Pre-Compositing:** Transition to `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL`
3. **Compositing:** Sample in fragment shader
4. **Presentation:** Swapchain image transitions to `VK_IMAGE_LAYOUT_PRESENT_SRC_KHR`

## Build Requirements

**Shader Compilation:**
Shaders must be compiled to SPIR-V before running. Use the Vulkan SDK:

```bash
# Install Vulkan SDK from https://vulkan.lunarg.com/
# Then run:
./scripts/compile_shaders.sh
```

Or manually:
```bash
glslc shaders/rt/raygen.rgen -o shaders/rt/raygen.rgen.spv
glslc shaders/rt/miss.rmiss -o shaders/rt/miss.rmiss.spv
glslc shaders/rt/closesthit.rchit -o shaders/rt/closesthit.rchit.spv
glslc shaders/basic/fullscreen.vert -o shaders/basic/fullscreen.vert.spv
glslc shaders/basic/fullscreen.frag -o shaders/basic/fullscreen.frag.spv
```

## Expected Behavior

When RT mode is enabled:
1. Ray tracing executes and writes sky gradient to storage image
2. Storage image is composited to swapchain via fullscreen quad
3. ImGui UI renders on top
4. Visible output: Sky gradient with sun and ImGui overlay

## Next Steps (Future Work)

To enable full ray tracing with geometry:

1. **Implement RTAccelerationStructure:**
   - `buildBLAS()` - Build bottom-level AS from world geometry
   - `buildTLAS()` - Build top-level AS referencing BLAS instances

2. **Add TLAS Descriptor Set:**
   - Create `set=0` with TLAS at `binding=0`
   - Update pipeline layout to include both `set=0` (TLAS) and `set=1` (storage image, camera UBO)
   - Bind both descriptor sets in `traceRays()`

3. **Restore Ray Tracing in Shaders:**
   - Uncomment/restore `traceRayEXT()` call in `raygen.rgen`
   - Add geometry buffers (vertices, indices, textures) to `closesthit.rchit`
   - Implement proper lighting and material sampling

4. **Extend Descriptor Sets:**
   - Add texture atlas binding for block textures
   - Add vertex/index buffer bindings for geometry access
   - Update shader descriptor layouts to match

## Validation

- Descriptor layouts match between shaders and pipeline
- No TLAS binding mismatches (removed from shaders for MVP)
- Image layout transitions are correct
- Compositing pipeline samples RT output correctly
- ImGui renders in same pass as compositing
