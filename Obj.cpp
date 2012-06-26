#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

#if defined(WIN32)
#  pragma warning(disable:4996)
#  include "glew.h"
#  include <GL/gl.h>
#elif defined(__APPLE__) || defined(MACOSX)
#  include <OpenGL/gl.h>
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/gl.h>
#endif

#include "Obj.h"

typedef GLfloat Vec[3];  // ベクトルの型
typedef GLuint Idx[3];   // 三角形の頂点インデックスの型
typedef Vec Tri[3];      // 頂点バッファオブジェクトのデータ構造

/*
** ファイルの読み込み
*/
bool Obj::load(const char *name)
{
  /* ファイルを開く */
  std::ifstream file(name, std::ios::binary);
  if (!file) {
    std::cerr << name << " が開けません" << std::endl;
    return false;
  }

  /* 頂点の数と面の数を調べる */
  char buf[1024];
  int v = 0, f = 0;

  while (file.getline(buf, sizeof buf)) {
    if (buf[0] == 'v' && buf[1] == ' ') {
      ++v;
    }
    else if (buf[0] == 'f' && buf[1] == ' ') {
      ++f;
    }
  }

  /* 作業用のメモリを確保する */
  Vec *vert = 0, *norm = 0, *fnorm = 0;
  Idx *face = 0;

  try {
    vert = new Vec[v];
    norm = new Vec[v];
    fnorm = new Vec[f];
    face = new Idx[f];
  }
  catch (std::bad_alloc) {
    std::cerr << "メモリが足りません" << std::endl;
  }

  /*
  ** 座標値の読み出しと法線ベクトルの計算
  */
  bool status = vert != 0 && norm != 0 && fnorm != 0 && face != 0;
  if (status) {
    /* 三角形の数 */
    faces = f;

    /* ファイルの巻き戻す */
    file.clear();
    file.seekg(0L, std::ios::beg);

    /* データを読み込む */
    v = f = 0;
    while (file.getline(buf, sizeof buf)) {
      if (buf[0] == 'v' && buf[1] == ' ') {
        sscanf(buf, "%*s %f %f %f", vert[v], vert[v] + 1, vert[v] + 2);
        ++v;
      }
      else if (buf[0] == 'f' && buf[1] == ' ') {
        if (sscanf(buf + 2, "%d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", face[f], face[f] + 1, face[f] + 2) != 3) {
          if (sscanf(buf + 2, "%d//%*d %d//%*d %d//%*d", face[f], face[f] + 1, face[f] + 2) != 3) {
            sscanf(buf + 2, "%d %d %d", face[f], face[f] + 1, face[f] + 2);
          }
        }
        --face[f][0];
        --face[f][1];
        --face[f][2];
        ++f;
      }
    }

    /* 頂点バッファオブジェクトをプログラムのメモリにマップ */
    glBindBuffer(GL_ARRAY_BUFFER, bufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof (Tri) * faces * 2, 0, GL_STATIC_DRAW);
    Tri *tri = (Tri *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    /* 面の法線ベクトルを算出する */
    for (int i = 0; i < f; ++i) {
      float x0 = vert[face[i][0]][0];
      float y0 = vert[face[i][0]][1];
      float z0 = vert[face[i][0]][2];
      float dx1 = vert[face[i][1]][0] - x0;
      float dy1 = vert[face[i][1]][1] - y0;
      float dz1 = vert[face[i][1]][2] - z0;
      float dx2 = vert[face[i][2]][0] - x0;
      float dy2 = vert[face[i][2]][1] - y0;
      float dz2 = vert[face[i][2]][2] - z0;

      fnorm[i][0] = dy1 * dz2 - dz1 * dy2;
      fnorm[i][1] = dz1 * dx2 - dx1 * dz2;
      fnorm[i][2] = dx1 * dy2 - dy1 * dx2;

      /* 基準の頂点 (p0) と, そこから２頂点に向かうベクトル (e1, e2) を VBO に格納する */
      (*tri)[0][0] = x0;
      (*tri)[0][1] = y0;
      (*tri)[0][2] = z0;

      (*tri)[1][0] = dx1;
      (*tri)[1][1] = dy1;
      (*tri)[1][2] = dz1;

      (*tri)[2][0] = dx2;
      (*tri)[2][1] = dy2;
      (*tri)[2][2] = dz2;

      ++tri;
    }

    /* 頂点の仮想法線ベクトルを算出する */
    for (int i = 0; i < v; ++i) {
      norm[i][0] = norm[i][1] = norm[i][2] = 0.0;
    }

    for (int i = 0; i < f; ++i) {
      norm[face[i][0]][0] += fnorm[i][0];
      norm[face[i][0]][1] += fnorm[i][1];
      norm[face[i][0]][2] += fnorm[i][2];

      norm[face[i][1]][0] += fnorm[i][0];
      norm[face[i][1]][1] += fnorm[i][1];
      norm[face[i][1]][2] += fnorm[i][2];

      norm[face[i][2]][0] += fnorm[i][0];
      norm[face[i][2]][1] += fnorm[i][1];
      norm[face[i][2]][2] += fnorm[i][2];
    }

    /* 頂点の仮想法線ベクトルを正規化する */
    for (int i = 0; i < v; ++i) {
      float a = sqrt(norm[i][0] * norm[i][0]
                   + norm[i][1] * norm[i][1]
                   + norm[i][2] * norm[i][2]);

      if (a != 0.0) {
        norm[i][0] /= a;
        norm[i][1] /= a;
        norm[i][2] /= a;
      }
    }

    /* 頂点の法線ベクトル (v0, v1, v2) を VBO に格納する */
    for (int i = 0; i < f; ++i) {
      (*tri)[0][0] = norm[face[i][0]][0];
      (*tri)[0][1] = norm[face[i][0]][1];
      (*tri)[0][2] = norm[face[i][0]][2];

      (*tri)[1][0] = norm[face[i][1]][0];
      (*tri)[1][1] = norm[face[i][1]][1];
      (*tri)[1][2] = norm[face[i][1]][2];

      (*tri)[2][0] = norm[face[i][2]][0];
      (*tri)[2][1] = norm[face[i][2]][1];
      (*tri)[2][2] = norm[face[i][2]][2];

      ++tri;
    }

    /* 頂点バッファオブジェクトのメモリをプログラムのメモリ空間から切り離す */
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  if (vert != 0) delete[] vert;
  if (norm != 0) delete[] norm;
  if (fnorm != 0) delete[] fnorm;
  if (face != 0) delete[] face;

  return status;
}

/*
** シェーダプログラムの登録
*/
void Obj::shader(GLuint program)
{
  shaderObject = program;
  
  /* uniform 変数の場所を得る */
  scr = glGetUniformLocation(program, "scr");
  org = glGetUniformLocation(program, "org");
  dir = glGetUniformLocation(program, "dir");
  
  /* attribute 変数の場所を得る */
  p0 = glGetAttribLocation(program, "p0");
  e1 = glGetAttribLocation(program, "e1");
  e2 = glGetAttribLocation(program, "e2");
  v0 = glGetAttribLocation(program, "v0");
  v1 = glGetAttribLocation(program, "v1");
  v2 = glGetAttribLocation(program, "v2");
}

/*
** 図形の表示の前準備
*/
void Obj::setup() const
{
  /* シェーダプログラムを有効にする */
  glUseProgram(shaderObject);

  /* attribute 変数を有効にする */
  glEnableVertexAttribArray(p0);
  glEnableVertexAttribArray(e1);
  glEnableVertexAttribArray(e2);
  glEnableVertexAttribArray(v0);
  glEnableVertexAttribArray(v1);
  glEnableVertexAttribArray(v2);

  /* 頂点バッファオブジェクトを有効にする */
  glBindBuffer(GL_ARRAY_BUFFER, bufferObject);

  /* attribute 変数にバッファオブジェクトの場所を指定する */
  glVertexAttribPointer(p0, 3, GL_FLOAT, GL_FALSE, sizeof (Tri), (GLubyte *)0 + sizeof (Vec) * 0);
  glVertexAttribPointer(e1, 3, GL_FLOAT, GL_FALSE, sizeof (Tri), (GLubyte *)0 + sizeof (Vec) * 1);
  glVertexAttribPointer(e2, 3, GL_FLOAT, GL_FALSE, sizeof (Tri), (GLubyte *)0 + sizeof (Vec) * 2);
  glVertexAttribPointer(v0, 3, GL_FLOAT, GL_FALSE, sizeof (Tri), (GLubyte *)0 + sizeof (Vec) * 0 + sizeof (Tri) * faces);
  glVertexAttribPointer(v1, 3, GL_FLOAT, GL_FALSE, sizeof (Tri), (GLubyte *)0 + sizeof (Vec) * 1 + sizeof (Tri) * faces);
  glVertexAttribPointer(v2, 3, GL_FLOAT, GL_FALSE, sizeof (Tri), (GLubyte *)0 + sizeof (Vec) * 2 + sizeof (Tri) * faces);
}

/*
** 図形の表示
*/
void Obj::draw(const float *screen, const float *origin, const float *direction) const
{
  /* uniform 変数に値を設定する */
  glUniform2fv(scr, 1, screen);
  glUniform3fv(org, 1, origin);
  glUniform3fv(dir, 1, direction);

  /* 三角形の数だけ点を描く */
  glDrawArrays(GL_POINTS, 0, faces);
}

/*
** 図形表示の後始末
*/
void Obj::cleanup() const
{
  /* 頂点バッファオブジェクトを無効にする */
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  /* attribute 変数を無効にする */
  glDisableVertexAttribArray(p0);
  glDisableVertexAttribArray(e1);
  glDisableVertexAttribArray(e2);
  glDisableVertexAttribArray(v0);
  glDisableVertexAttribArray(v1);
  glDisableVertexAttribArray(v2);

  /* シェーダプログラムを無効にする */
  glUseProgram(0);
}
