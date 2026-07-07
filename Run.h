#pragma once

// 構造体の宣言
struct OBJECT	// 自機や敵機用
{
	int x;	// x座標
	int y;	// y座標
	int vx;	// x軸方向の速さ
	int vy;	// y軸方向の速さ
	int state;	// 存在するか
	int pattern;	// 敵機の動きのパターン
	int image;		// 画像
	int wid;		// 画像の幅（ピクセル数）
	int hei;		// 画像の高さ
	int timer;		// タイマー
};

// 関数プロトタイプ宣言
// ここにプロトタイプ宣言を記述する
void InitGame(void);
void MovePlayer(void);
int LoadGraphWithCheck(const char* file);
int LoadSoundMemWithCheck(const char* file);
void ScrollBG(int spd);
int SetEnemy(int x, int y, int vx, int vy, int ptn, int img);
int SetTrap(int x, int y, int vx, int vy, int ptn, int img);
void MoveEnemy(void);
void SetItem(void);
void MoveItem(void);
void MoveTrap(void);
void CheckCollision(void);
void DrawUI(void);
void DrawTextC(int x, int y, const char* txt, int col, int siz);
void InitVariable(void);