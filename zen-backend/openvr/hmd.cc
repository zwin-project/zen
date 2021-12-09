#include "hmd.h"

#include <cglm/cglm.h>
#include <libzen-compositor/libzen-compositor.h>
#include <openvr/openvr.h>

Hmd::Hmd() {}

bool
Hmd::Init()
{
  vr::EVRInitError init_error = vr::VRInitError_None;
  vr_system_ = vr::VR_Init(&init_error, vr::VRApplication_Scene);

  if (init_error != vr::VRInitError_None) {
    zen_log("Hmd: failed to init OpenVR: %s\n",
        vr::VR_GetVRInitErrorAsEnglishDescription(init_error));
    goto err;
  }

  if (!vr::VRCompositor()) {
    zen_log("Hmd: failed to init VR Compositor\n");
    goto err_compositor;
  }

  vr_system_->GetRecommendedRenderTargetSize(&display_width_, &display_height_);

  glm_mat4_identity(head_pose_);

  if (!InitEye(kLeftEye)) {
    zen_log("Hmd: failed to init a left eye\n");
    goto err_left_eye;
  }

  if (!InitEye(kRightEye)) {
    zen_log("Hmd: failed to init a right eye\n");
    goto err_right_eye;
  }

  if (!InitPreview()) {
    zen_log("Hmd: failed to init preview\n");
    goto err_preview;
  }

  return true;

err_preview:
  DeinitEye(kRightEye);

err_right_eye:
  DeinitEye(kLeftEye);

err_left_eye:
err_compositor:
  vr::VR_Shutdown();

err:
  return false;
}

Hmd::~Hmd()
{
  DeinitEye(kLeftEye);
  DeinitEye(kRightEye);
  DeinitPreview();
  vr::VR_Shutdown();
}

void
Hmd::GetCameras(struct zen_opengl_renderer_camera cameras[2])
{
  for (int i = 0; i < 2; i++) {
    cameras[i].framebuffer_id = eyes_[i].framebuffer_id_;

    glm_mat4_copy(eyes_[i].projection_matrix_, cameras[i].projection_matrix);

    glm_mat4_copy(head_pose_, cameras[i].view_matrix);
    glm_mat4_mul(eyes_[i].head_to_view_matrix_, cameras[i].view_matrix,
        cameras[i].view_matrix);

    cameras[i].viewport.width = display_width_;
    cameras[i].viewport.height = display_height_;
    cameras[i].viewport.x = 0;
    cameras[i].viewport.y = 0;
  }
}

void
Hmd::DrawPreview(GLuint framebuffer, uint32_t view_width, uint32_t view_height)
{
  int x, y, width, height;
  if (view_width * display_height_ > display_width_ * 2 * view_height) {
    height = view_height;
    width = display_width_ * 2 * view_height / display_height_;
    x = (view_width - width) / 2;
    y = 0;
  } else {
    width = view_width;
    height = display_height_ * view_width / (display_width_ * 2);
    x = 0;
    y = (view_height - height) / 2;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glDisable(GL_DEPTH_TEST);
  glViewport(x, y, width, height);
  glClearColor(0.4, 0.4, 0.4, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glBindVertexArray(preview_.vertex_array_);
  glUseProgram(preview_.shader_);

  glBindTexture(GL_TEXTURE_2D, eyes_[kLeftEye].resolve_texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glDrawElements(GL_TRIANGLES, preview_.index_count_ / 2, GL_UNSIGNED_SHORT, 0);

  glBindTexture(GL_TEXTURE_2D, eyes_[kRightEye].resolve_texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glDrawElements(GL_TRIANGLES, preview_.index_count_ / 2, GL_UNSIGNED_SHORT,
      (const void *)(uintptr_t)(sizeof(unsigned short) * preview_.index_count_ /
                                2));

  glBindVertexArray(0);
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int
Hmd::CompileShader(
    const char *vertex_shader_source, const char *fragment_shader_source)
{
  GLuint id = glCreateProgram();

  // compile vertex shader
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
  glCompileShader(vertex_shader);

  GLint vertex_shader_compiled = GL_FALSE;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_compiled);
  if (vertex_shader_compiled != GL_TRUE) {
    zen_log("opengl shader compiler: failed to compile vertex shader\n");
    glDeleteProgram(id);
    glDeleteShader(vertex_shader);
    return -1;
  }
  glAttachShader(id, vertex_shader);
  glDeleteShader(vertex_shader);

  // compile fragment shader
  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
  glCompileShader(fragment_shader);

  GLint fragment_shader_compiled = GL_FALSE;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_shader_compiled);
  if (fragment_shader_compiled != GL_TRUE) {
    zen_log("opengl shader compiler: failed to compile fragment shader\n");
    glDeleteProgram(id);
    glDeleteShader(fragment_shader);
    return -1;
  }
  glAttachShader(id, fragment_shader);
  glDeleteShader(vertex_shader);

  glLinkProgram(id);

  GLint program_success = GL_FALSE;
  glGetProgramiv(id, GL_LINK_STATUS, &program_success);
  if (program_success != GL_TRUE) {
    zen_log("opengl shader compiler: failed to link program\n");
    glDeleteProgram(id);
    return -1;
  }

  glUseProgram(id);
  glUseProgram(0);

  return id;
}

bool
Hmd::InitEye(HmdEye eye)
{
  glGenFramebuffers(1, &eyes_[eye].framebuffer_id_);
  glBindFramebuffer(GL_FRAMEBUFFER, eyes_[eye].framebuffer_id_);

  glGenRenderbuffers(1, &eyes_[eye].depth_buffer_id_);
  glBindRenderbuffer(GL_RENDERBUFFER, eyes_[eye].depth_buffer_id_);
  glRenderbufferStorageMultisample(
      GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, display_width_, display_height_);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
      GL_RENDERBUFFER, eyes_[eye].depth_buffer_id_);

  glGenTextures(1, &eyes_[eye].texture_id_);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, eyes_[eye].texture_id_);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8,
      display_width_, display_height_, true);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
      GL_TEXTURE_2D_MULTISAMPLE, eyes_[eye].texture_id_, 0);

  glGenFramebuffers(1, &eyes_[eye].resolve_framebuffer_id_);
  glBindFramebuffer(GL_FRAMEBUFFER, eyes_[eye].resolve_framebuffer_id_);

  glGenTextures(1, &eyes_[eye].resolve_texture_id_);
  glBindTexture(GL_TEXTURE_2D, eyes_[eye].resolve_texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, display_width_, display_height_, 0,
      GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      eyes_[eye].resolve_texture_id_, 0);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    this->DeinitEye(eye);
    return false;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  UpdateProjectionMatrix(eye);
  UpdateHeadToViewMatrix(eye);

  return true;
}

void
Hmd::DeinitEye(HmdEye eye)
{
  glDeleteFramebuffers(1, &eyes_[eye].framebuffer_id_);
  glDeleteRenderbuffers(1, &eyes_[eye].depth_buffer_id_);
  glDeleteTextures(1, &eyes_[eye].texture_id_);
  glDeleteBuffers(1, &eyes_[eye].resolve_framebuffer_id_);
  glDeleteTextures(1, &eyes_[eye].resolve_texture_id_);
}

bool
Hmd::InitPreview()
{
  int32_t shader_id = CompileShader(
      // vertex shader
      "#version 410 core\n"
      "layout(location = 0) in vec4 position;\n"
      "layout(location = 1) in vec2 v2UVIn;\n"
      "noperspective out vec2 v2UV;\n"
      "void main()\n"
      "{\n"
      "	v2UV = v2UVIn;\n"
      "	gl_Position = position;\n"
      "}\n",

      // fragment shader
      "#version 410 core\n"
      "uniform sampler2D userTexture;\n"
      "noperspective in vec2 v2UV;\n"
      "out vec4 outputColor;\n"
      "void main()\n"
      "{\n"
      "  outputColor = texture(userTexture, v2UV);\n"
      "}\n");
  if (shader_id < 0) return false;

  preview_.shader_ = shader_id;

  float vertices[8][4] = {
      // left eye
      {-1, -1, 0, 0},  // {x, y, u, v}
      {+0, -1, 1, 0},
      {-1, +1, 0, 1},
      {+0, +1, 1, 1},
      // right eye
      {+0, -1, 0, 0},
      {+1, -1, 1, 0},
      {+0, +1, 0, 1},
      {+1, +1, 1, 1},
  };

  GLushort indices[] = {0, 1, 3, 0, 3, 2, 4, 5, 7, 4, 7, 6};
  preview_.index_count_ = sizeof(indices) / sizeof(indices[0]);

  glGenVertexArrays(1, &preview_.vertex_array_);
  glBindVertexArray(preview_.vertex_array_);

  glGenBuffers(1, &preview_.vertex_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, preview_.vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &preview_.element_array_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, preview_.element_array_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
      preview_.index_count_ * sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
      1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void *)(sizeof(float) * 2));

  glBindVertexArray(0);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  return true;
}

void
Hmd::DeinitPreview()
{
  glDeleteProgram(preview_.shader_);
  glDeleteBuffers(1, &preview_.vertex_buffer_);
  glDeleteBuffers(1, &preview_.element_array_);
  glDeleteVertexArrays(1, &preview_.vertex_array_);
}

void
Hmd::WaitUpdatePoses()
{
  vr::TrackedDevicePose_t
      tracked_device_pose_list[vr::k_unMaxTrackedDeviceCount];
  vr::TrackedDevicePose_t vr_hmd_pose;

  vr::VRCompositor()->WaitGetPoses(
      tracked_device_pose_list, vr::k_unMaxTrackedDeviceCount, NULL, 0);

  vr_hmd_pose = tracked_device_pose_list[vr::k_unTrackedDeviceIndex_Hmd];
  if (vr_hmd_pose.bPoseIsValid) {
    vr::HmdMatrix34_t mat = vr_hmd_pose.mDeviceToAbsoluteTracking;

    mat4 tmp = {
        mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0f,  //
        mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0f,  //
        mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0f,  //
        mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f   //
    };

    glm_mat4_inv_precise(tmp, head_pose_);
  }
}

void
Hmd::Submit()
{
  for (int i = 0; i < 2; i++) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, eyes_[i].framebuffer_id_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, eyes_[i].resolve_framebuffer_id_);
    glBlitFramebuffer(0, 0, display_width_, display_height_, 0, 0,
        display_width_, display_height_, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  }

  vr::Texture_t left_eye_texture = {
      (void *)(uintptr_t)eyes_[0].resolve_texture_id_,
      vr::TextureType_OpenGL,
      vr::ColorSpace_Gamma,
  };
  vr::VRCompositor()->Submit(vr::Eye_Left, &left_eye_texture);

  vr::Texture_t right_eye_texture = {
      (void *)(uintptr_t)eyes_[1].resolve_texture_id_,
      vr::TextureType_OpenGL,
      vr::ColorSpace_Gamma,
  };
  vr::VRCompositor()->Submit(vr::Eye_Right, &right_eye_texture);
}

void
Hmd::UpdateProjectionMatrix(HmdEye eye)
{
  float nearClip = 0.1f;
  float farClip = 10000.0f;
  vr::Hmd_Eye hmd_eye = eye == kLeftEye ? vr::Eye_Left : vr::Eye_Right;
  vr::HmdMatrix44_t mat =
      vr_system_->GetProjectionMatrix(hmd_eye, nearClip, farClip);

  mat4 tmp = {
      mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],  //
      mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],  //
      mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],  //
      mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]   //
  };

  glm_mat4_copy(tmp, eyes_[eye].projection_matrix_);
}

void
Hmd::UpdateHeadToViewMatrix(HmdEye eye)
{
  vr::Hmd_Eye hmd_eye = eye == kLeftEye ? vr::Eye_Left : vr::Eye_Right;
  vr::HmdMatrix34_t mat = vr_system_->GetEyeToHeadTransform(hmd_eye);

  mat4 tmp = {
      mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0f,  //
      mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0f,  //
      mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0f,  //
      mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f   //
  };

  glm_mat4_inv_precise(tmp, eyes_[eye].head_to_view_matrix_);
}
