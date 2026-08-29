//
// main.cpp
//
#if defined(_WIN32)
#  define _USE_MATH_DEFINES
#  define _CRT_SECURE_NO_WARNINGS
#  include <GL/glew.h>
#  include <GL/glut.h>
#elif defined(__APPLE__) || defined(MACOSX)
#  define GL_SILENCE_DEPRECATION
#  include <GLUT/glut.h>
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/glut.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <cmath>

/*
** 形状データ
*/
#include "Obj.h"
static const char objectData[] = "bunny_s.obj";
static Obj *obj = 0;

/*
** シェーダ
*/
#include "shadersource.h"
static const char vertSource[] = "simple.vert";
static const char fragSource[] = "simple.frag";

/*
** カメラ
*/
static const float cameraPosition[] = { 0.0f, 0.0f, 200.0f };
static const float cameraTarget[] = { 0.0f, 0.0f, 0.0f };
static const float cameraUp[] = { 0.0f, 1.0f, 0.0f };
static const float cameraFov = 60.0f;
static const float cameraNear = 1.0f;
static const float cameraFar = 1000.0f;

/*
** ウィンドウサイズ
*/
static int width, height;

/*
** 視線の向き
*/
static void lookat(const float *e, const float *t, const float *u, float *m)
{
  float tx, ty, tz, l;

  /* z 軸 = e - t */
  tx = e[0] - t[0];
  ty = e[1] - t[1];
  tz = e[2] - t[2];
  l = sqrt(tx * tx + ty * ty + tz * tz);
  if (l > 0.0f) {
    m[6] = tx / l;
    m[7] = ty / l;
    m[8] = tz / l;
  }

  /* x 軸 = u x z 軸 */
  tx = u[1] * m[8] - u[2] * m[7];
  ty = u[2] * m[6] - u[0] * m[8];
  tz = u[0] * m[7] - u[1] * m[6];
	l = sqrt(tx * tx + ty * ty + tz * tz);
  if (l > 0.0f) {
    m[0] = tx / l;
    m[1] = ty / l;
    m[2] = tz / l;
  }

  /* y 軸 = z 軸 x x 軸 */
  m[3] = m[7] * m[2] - m[8] * m[1];
  m[4] = m[8] * m[0] - m[6] * m[2];
  m[5] = m[6] * m[1] - m[7] * m[0];
}

/*
** ベクトルと行列の積 (b ← a x m)
*/
static void projection(float *b, const float *a, const float *m)
{
  b[0] = a[0] * m[0] + a[1] * m[3] + a[2] * m[6];
  b[1] = a[0] * m[1] + a[1] * m[4] + a[2] * m[7];
  b[2] = a[0] * m[2] + a[1] * m[5] + a[2] * m[8];
}

/*
** 画面表示
*/
static void display(void)
{
  /* 開始時刻の取得 */
  int beginTime = glutGet(GLUT_ELAPSED_TIME);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  
#if 1 /* スクリーンを回転する場合 */
  float axs[9];         // スクリーン空間の軸ベクトル
  axs[1] = axs[3] = axs[6] = axs[7] = 0.0f;
  axs[2] = axs[5] = axs[8] = (cameraNear - cameraFar) * 0.5f;
  axs[4] = -axs[8] * tan(cameraFov * 0.5f * 3.1415926f / 180.0f);
  axs[0] = axs[4] * (float)width / (float)height;

  float h;              // 視点から前方面までの距離の割合
  h = 1.0f - cameraNear / axs[8];
  
  float rot[9];         // 視線の方向を回転する行列
  lookat(cameraPosition, cameraTarget, cameraUp, rot);

  float viw[9];         // 視線の方向に回転したスクリーン空間の軸ベクトル
  projection(viw + 0, axs + 0, rot);
  projection(viw + 3, axs + 3, rot);
  projection(viw + 6, axs + 6, rot);
  
  float scr[3];         // スクリーン上の点のパラメータ座標 (u, v, w)
  
  obj->setup();
  for (int y = 0; y < height; ++y) {
    scr[1] = (float)(y + y + 1) / (float)height - 1.0f;
    
    for (int x = 0; x < width; ++x) {
      scr[0] = (float)(x + x + 1) / (float)width - 1.0f;
      scr[2] = 1.0f - scr[0] - scr[1];
      
      float dir[3];     // ワールド空間における視線の方向
      projection(dir, scr, viw);
      
      float org[3] = {  // 前方面上にある視線の出発点 (≠視点位置)
        dir[0] * h + cameraPosition[0],
        dir[1] * h + cameraPosition[1],
        dir[2] * h + cameraPosition[2],
      };
      
      /* 点の描画 */
      obj->draw(scr, org, dir);
    }
    glFinish();
  }
  obj->cleanup();
#else /* 視線を回転する場合 */
  float ray[3];         // 視野空間における視線の方向
  ray[2] = (cameraNear - cameraFar) * 0.5f;

  float sx, sy;         // スクリーンのサイズ
  sy = -ray[2] * tan(cameraFov * 0.5f * 3.1415926f / 180.0f);
  sx = sy * (float)height / (float)width;

  float h;              // 視点と前方面との距離
  h = 1.0f - cameraNear / ray[2];

  float rot[9];         // 視線の方向を回転する行列
  lookat(cameraPosition, cameraTarget, cameraUp, rot);
  
  float scr[2];         // スクリーン上の点の相対位置

  obj->setup();
  for (int y = 0; y < height; ++y) {
    scr[1] = (float)(y + y + 1) / (float)height - 1.0f;
    ray[1] = scr[1] * sy;

    for (int x = 0; x < width; ++x) {
      scr[0] = (float)(x + x + 1) / (float)width - 1.0f;
      ray[0] = scr[0] * sx;

      float dir[3];     // ワールド空間における視線の方向
      projection(dir, ray, rot);

      float org[3] = {  // 前方面上の視線の出発点 (≠視点位置)
        dir[0] * h + cameraPosition[0],
        dir[1] * h + cameraPosition[1],
        dir[2] * h + cameraPosition[2],
      };

      /* 点の描画 */
      obj->draw(scr, org, dir);
    }
    glFinish();
  }
  obj->cleanup();
#endif
  
  glDisable(GL_DEPTH_TEST);
  
  /* 経過時間の表示 */
  fprintf(stderr, "Elapsed time = %f\n", (double)(glutGet(GLUT_ELAPSED_TIME) - beginTime) * 0.001);
}

/*
** ウィンドウサイズ
*/
static void resize(int w, int h)
{
  /* ウィンドウの幅と高さを保存する */
  width = w;
  height = h;
  
  /* ウィンドウ全体をビューポートにする */
  glViewport(0, 0, w, h);
}

/*
** キーボード
*/
static void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
    /* q か Q か ESC をタイプしたら終了 */
    case '\033':
    case 'q':
    case 'Q':
      exit(0);
    case 'g':
    case 'G':
      /* g か G をタイプしたら再描画 */
      glutPostRedisplay();
    default:
      break;
  }
}

/*
** シェーダプログラムの読み込み
*/
static GLuint loadShader(const char *vertSource, const char *fragSource)
{
  /* シェーダオブジェクトの作成 */
  GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  
  /* シェーダのソースプログラムの読み込み */
  if (readShaderSource(vertShader, vertSource)) return 0;
  if (readShaderSource(fragShader, fragSource)) return 0;
  
  /* シェーダプログラムのコンパイル／リンク結果を得る変数 */
  GLint status;
  
  /* バーテックスシェーダのソースプログラムのコンパイル */
  glCompileShader(vertShader);
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &status);
  printShaderInfoLog(vertShader);
  if (status == GL_FALSE) {
    fprintf(stderr, "Compile error in %s.\n", vertSource);
    return 0;
  }
  
  /* フラグメントシェーダのソースプログラムのコンパイル */
  glCompileShader(fragShader);
  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);
  printShaderInfoLog(fragShader);
  if (status == GL_FALSE) {
    fprintf(stderr, "Compile error in %s.\n", fragSource);
    return 0;
  }
  
  /* プログラムオブジェクトの作成 */
  GLuint gl2Program = glCreateProgram();
  
  /* シェーダオブジェクトのシェーダプログラムへの登録 */
  glAttachShader(gl2Program, vertShader);
  glAttachShader(gl2Program, fragShader);
  
  /* シェーダオブジェクトの削除 */
  glDeleteShader(vertShader);
  glDeleteShader(fragShader);
  
  /* シェーダプログラムのリンク */
  glLinkProgram(gl2Program);
  glGetProgramiv(gl2Program, GL_LINK_STATUS, &status);
  printProgramInfoLog(gl2Program);
  if (status == GL_FALSE) {
    fprintf(stderr, "Link error.\n");
    return 0;
  }
  
  return gl2Program;
}

/*
** 後始末
*/
static void cleanup(void)
{
  delete obj;
}

/*
** 初期化
*/
static void init(void)
{
#if defined(_WIN32)
  /* GLEW の初期化 */
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    exit(1);
  }
#endif
  /* シェーダプログラムの読み込み */
  GLuint shader = loadShader(vertSource, fragSource);
  if (shader == 0) exit(1);
  
  /* 形状データの読み込み */
  obj = new Obj;
  if (obj == 0) exit(1);
  atexit(cleanup);
  if (!obj->load(objectData)) exit(1);
  obj->shader(shader);

  /* 背景色 */
  glClearColor(0.0, 0.1, 0.3, 1.0);
  
  /* 経過時間の初期化 */
  glutGet(GLUT_ELAPSED_TIME);
}

/*
** メインプログラム
*/
int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutKeyboardFunc(keyboard);
  init();
  glutMainLoop();
  
  return 0;
}
