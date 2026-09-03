# winter03

## 1. 概要

このプログラムは、OpenGL と GLSL バーテックスシェーダ・フラグメントシェーダを用いて、Möller–Trumbore の交差判定アルゴリズムによるレイ・三角形交差判定を GPU 上で実行し、ポリゴンモデルのレイキャスティングレンダリングを行うサンプルプログラムです。本プログラムは、以下のブログ記事の解説に対応しています。

- [第１７回 レイキャスティング](https://tokoik.github.io/blog/2010-01-13.html)

各三角形の頂点位置・法線ベクトル情報を VBO に格納し、画面上の画素ごとにシーン内の全三角形に対して視線との交差判定をバーテックスシェーダで計算してデプスバッファにより最前面の交点を抽出します。

## 2. 対応環境

- **Windows**: Visual Studio 2019 以降 / CMake 3.22 以降
- **macOS**: Xcode 12 以降 / CMake 3.22 以降
- **Linux (Ubuntu 等)**: GCC / Clang / CMake 3.22 以降

## 3. ビルド手順

### 3.1 Windows (Visual Studio)

```powershell
cmake -B build
cmake --build build --config Release
```

### 3.2 Linux (Ubuntu)

必要なパッケージをインストールした上でビルドします。

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev libglew-dev
cmake -B build
cmake --build build
```

### 3.3 macOS (Xcode)

```bash
cmake -B build -G Xcode
cmake --build build --config Release
```

## 4. 起動方法

ビルド完了後、生成された実行ファイルを実行します。

```powershell
./build/Release/winter03.exe
```

## 5. 操作方法

- 起動すると、モデルデータ（`bunny_s.obj`）のレイキャスティング描画が開始されます。
- 画面サイズ（300x300）の各画素ごとに走査線単位でレンダリングが進行します。
- ウィンドウの閉じるボタンまたはコンソールで `Ctrl+C` を押すとプログラムを終了します。

## 6. プログラムの解説

### 6.1 Möller–Trumbore の交差判定アルゴリズム

視線の基点を $\mathbf{O}$、方向を $\mathbf{D}$ とし、三角形の頂点を $\mathbf{P}_0, \mathbf{P}_1, \mathbf{P}_2$ とすると、交点パラメータ $(t, u, v)$ は以下のように求められます。

$$
\begin{pmatrix}
t \\
u \\
v
\end{pmatrix}
=
\frac{1}{\mathbf{Q} \cdot \mathbf{E}_1}
\begin{pmatrix}
\mathbf{R} \cdot \mathbf{E}_2 \\
\mathbf{Q} \cdot \mathbf{E} \\
\mathbf{R} \cdot \mathbf{D}
\end{pmatrix}
$$

ここで、$\mathbf{E}_1 = \mathbf{P}_1 - \mathbf{P}_0$, $\mathbf{E}_2 = \mathbf{P}_2 - \mathbf{P}_0$, $\mathbf{E} = \mathbf{O} - \mathbf{P}_0$、$\mathbf{Q} = \mathbf{D} \times \mathbf{E}_2$、$\mathbf{R} = \mathbf{E} \times \mathbf{E}_1$ です。

### 6.2 バーテックスシェーダ (`simple.vert`)

バーテックスシェーダ内で上記の交差判定を行い、求めた交点距離 $t$ をデプス値として `gl_Position.z` に設定します。これにより、デプスバッファテストによって視点に最も近い交点が選択されます。

### 6.3 フラグメントシェーダ (`simple.frag`)

パラメータ $u, v$ から $w = 1 - u - v$ を求め、$(u, v, w)$ のいずれかが負であれば三角形の外側であるため `discard` でフラグメントを破棄します。三角形内の場合は法線ベクトルを補間して簡易的な陰影付けを行います。
