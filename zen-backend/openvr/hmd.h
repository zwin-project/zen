#ifndef ZEN_OPENVR_BACKEND_HMD_H
#define ZEN_OPENVR_BACKEND_HMD_H

#include <GL/glew.h>
#include <cglm/cglm.h>
#include <openvr/openvr.h>
#include <zen-renderer/opengl-renderer.h>

class Hmd
{
 public:
  enum HmdEye {
    kLeftEye = 0,
    kRightEye = 1,
    kPreviewEye = 2,
  };

 public:
  Hmd(struct zen_config *config);
  bool Init();
  ~Hmd();
  void WaitUpdatePoses();
  void Submit();
  void DrawPreview(
      GLuint framebuffer, uint32_t view_width, uint32_t view_height);
  int GetCameras(struct zen_opengl_renderer_camera *cameras);
  void GetHeadPosition(vec3 position);

 private:
  vr::IVRSystem *vr_system_;
  uint32_t display_width_;
  uint32_t display_height_;
  uint32_t preview_width_;    /* for better preview */
  uint32_t preview_height_;   /* for better preview */
  uint32_t preview_distance_; /* for better preview */
  mat4 head_pose_;
  const bool better_preview_;

  struct {
    GLuint framebuffer_id_;
    GLuint texture_id_;
    GLuint depth_buffer_id_;
    GLuint resolve_framebuffer_id_;
    GLuint resolve_texture_id_;
    mat4 projection_matrix_;
    mat4 head_to_view_matrix_;
  } eyes_[3];

  struct {
    GLuint vertex_array_;
    GLuint element_array_;
    GLuint vertex_buffer_;
    GLuint shader_;
    size_t index_count_;
  } preview_;

 private:
  int32_t CompileShader(
      const char *vertex_shader_source, const char *fragment_shader_source);
  bool InitEye(HmdEye eye, uint32_t width, uint32_t height);
  void DeinitEye(HmdEye eye);
  bool InitPreview();
  void DeinitPreview();
  void UpdateProjectionMatrix(HmdEye eye);
  void UpdateHeadToViewMatrix(HmdEye eye);
  void DrawBetterPreview(
      GLuint framebuffer, uint32_t view_width, uint32_t view_height);
  void DrawCopyPreview(
      GLuint framebuffer, uint32_t view_width, uint32_t view_height);
};

#endif  //  ZEN_OPENVR_BACKEND_HMD_H
