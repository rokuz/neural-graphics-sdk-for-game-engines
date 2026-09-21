/*
 * SPDX-License-Identifier: MIT
 */

// Binds the resources of the Neural Super Sampling shaders (`ffx_nss_*.h`) through a bindless descriptor set
// instead of the fixed `layout(set, binding)` declarations of the SDK. Enabled by `#define NSS_BINDLESS 1`.
//
// The NSS resources are mapped onto the host renderer's bindless arrays with indices coming from push constants (see
// `NssPushConstants` on the host side); the constant buffer `cbNSS` becomes a buffer reference.

#ifndef FFX_NSS_BINDLESS_H
#define FFX_NSS_BINDLESS_H

// `NssConstants` on the host side; the layout matches the `cbNSS_t` uniform block of the SDK (std430)
layout(std430, buffer_reference) readonly buffer NssConstantsRef {
  vec4 _DeviceToViewDepth;
  vec4 _JitterOffset;
  vec4 _JitterOffsetTm1;
  vec4 _ScaleFactor;
  ivec2 _OutputDims;
  ivec2 _InputDims;
  vec2 _InvOutputDims;
  vec2 _InvInputDims;
  ivec2 _DepthTm1Size;
  vec2 _InvDepthTm1Size;
  ivec2 _InputTensorSize;
  vec2 _InputTensorSizeRcp;
  ivec2 _KpnDimension;
  vec2 _MotionVectorScale;
  vec2 _PaddingScale;
  float _DepthClipRequiredSepScale;
  float _DepthClipPower;
  vec2 _KpnScale;
  int _DebugViewMode;
  float _NotHistoryReset;
  vec2 _Exposure;
  ivec2 _IndexModulo;
  ivec2 _ReducedInputModulo;
  ivec2 _LutOffset;
};

// `NssPushConstants` on the host side
layout(push_constant) uniform PushConstants {
  NssConstantsRef constants;
  uint samplerPoint; // s_PointClamp
  uint samplerLinear; // s_LinearClamp
  uint texColor; // r_input_color_jittered
  uint texDepth; // r_input_depth
  uint texMotion; // r_input_motion_vectors
  uint texHistory; // r_prev_upscaled_color
  uint texFeedback; // r_temporal_feedback_tensor: a texture aliasing the feedback tensor
  uint texLumaDerivTm1; // r_prev_luma_deriv
  uint texDepthTm1; // r_prev_depth (R32_UINT)
  uint texNearestDepthCoord; // r_input_nearest_depth_coord
  uint imgDepthTm1; // rw_reconstructed_prev_depth
  uint imgLumaDeriv; // rw_luma_deriv
  uint imgNearestDepthCoord; // rw_nearest_depth_coord_out
  uint imgOutput; // rw_upscaled_output
  uint tensorInput; // rw_preprocessed_tensor
  uint tensorKpn; // r_coefficients_kpn_tensor
} pc;

// The host renderer declares the bindless texture and sampler arrays for fragment shaders only: compute shaders declare their own
layout(set = 0, binding = 0) uniform texture2D kTextures2D[];
layout(set = 0, binding = 0) uniform utexture2D kTextures2DUint[];
layout(set = 0, binding = 1) uniform sampler kSamplers[];

// storage images are declared per format
layout(set = 0, binding = 2, r32ui) coherent uniform uimage2D kImagesR32UI[];
layout(set = 0, binding = 2, rgba8_snorm) uniform image2D kImagesRGBA8Snorm[];
layout(set = 0, binding = 2, r8) uniform image2D kImagesR8[];
layout(set = 0, binding = 2, rgba16f) uniform image2D kImagesRGBA16F[];

#define cbNSS pc.constants

#define s_PointClamp kSamplers[pc.samplerPoint]
#define s_LinearClamp kSamplers[pc.samplerLinear]

#define r_input_color_jittered kTextures2D[pc.texColor]
#define r_input_depth kTextures2D[pc.texDepth]
#define r_input_motion_vectors kTextures2D[pc.texMotion]
#define r_prev_upscaled_color kTextures2D[pc.texHistory]
#define r_temporal_feedback_tensor kTextures2D[pc.texFeedback]
#define r_prev_luma_deriv kTextures2D[pc.texLumaDerivTm1]
#define r_prev_depth kTextures2DUint[pc.texDepthTm1]
#define r_input_nearest_depth_coord kTextures2D[pc.texNearestDepthCoord]

#define rw_reconstructed_prev_depth kImagesR32UI[pc.imgDepthTm1]
#define rw_luma_deriv kImagesRGBA8Snorm[pc.imgLumaDeriv]
#define rw_nearest_depth_coord_out kImagesR8[pc.imgNearestDepthCoord]
#define rw_upscaled_output kImagesRGBA16F[pc.imgOutput]

// tensors: `kTensorsI8_4[]` is declared by the host renderer (`tensorARM<int8_t, 4>`)
#define rw_preprocessed_tensor kTensorsI8_4[pc.tensorInput]
#define r_coefficients_kpn_tensor kTensorsI8_4[pc.tensorKpn]

#endif // FFX_NSS_BINDLESS_H
