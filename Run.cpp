#include "DxLib.h"
#include "Run.h"
#include <math.h>
const int WIDTH = 1200, HEIGHT = 720;	// ウィンドウの幅と高さのピクセル数
const int FPS = 60;	// フレームレート
const int IMG_ENEMY_MAX = 4;	// 敵の画像の枚数
const int IMG_TRAP_MAX = 2;	// トラップの画像の枚数
const int ENEMY_MAX = 3;	// 敵の最大数
const int ITEM_MAX = 100;	// アイテムの最大数
const int TRAP_MAX = 2;	// トラップの最大数
const int RUN_MAX = 100;
enum { TITLE, PLAY, OVER};	// シーンを分けるための列挙定数

// グローバル変数
// ここでゲームに用いる変数や配列を定義する
int imgCastle, imgFloor, imgCeiling;	// 背景画像
int imgRun, imgJump, imgSliding, imgAttack, imgDamage;		// プレイヤーの画像
int imgEnemy[IMG_ENEMY_MAX];	// 敵の画像
int imgTrap[IMG_TRAP_MAX];	// トラップの画像
int imgDonut, imgLightning, imgMagnet;	// アイテムの画像
int bgm, seDonut, seLightning, seMagnet;		// 音の読み込み用
int distance = 0;	// ステージ終端までの距離
int stage = 1;	// ステージ
int timer = 0;	// タイマー
int score = 0;
int hp = 100;
int invincibleTimer = 0;
int magnetTimer = 0;
int playerY = 450;
int playerVY = 0;
int scene = TITLE;	// 現在のシーン
int attackTimer = 0;

bool invincibleMode = false;
bool magnetMode = false;
bool isJump = false;
bool attackMode = false;
struct OBJECT player;
struct OBJECT enemy[ENEMY_MAX];
struct OBJECT trap[TRAP_MAX];
struct OBJECT item[ITEM_MAX];
struct OBJECT run[RUN_MAX];	// エフェクト用の構造体の配列


int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	SetWindowText("Sweets Run");	// ウィンドウのタイトル
	SetGraphMode(WIDTH, HEIGHT, 32);	// ウィンドウの大きさとカラービット数の指定
	ChangeWindowMode(true);	// ウィンドウモードで起動
	if (DxLib_Init() == -1) return -1;	// ライブラリ初期化　エラーが起きたら終了
	SetBackgroundColor(0, 0, 0);	// 背景色の指定
	SetDrawScreen(DX_SCREEN_BACK);	// 描画面を裏画面にする

	InitGame();		//　初期化用の関数を呼び出す

	while (1)	// メインループ
	{
		ClearDrawScreen();	// 画面をクリアする
		int spd = 1;
		if (scene == PLAY && distance == 0) spd = 0;

			MoveEnemy();
			MoveTrap();
			MoveItem();
			CheckCollision();
			DrawUI();

		// ゲームの骨組みとなる処理を、ここに記入する
		ScrollBG(spd);	// 背景をスクロールさせる
		timer++;
		switch (scene)	// シーンごとに処理を分岐
		{
		case TITLE:	// タイトル画面
			DrawTextC(WIDTH * 0.5, HEIGHT * 0.3, "Sweets Run", 0xffffff, 80);
			DrawTextC(WIDTH * 0.5, HEIGHT * 0.7, "Press Space To Start", 0xffffff, 30);
			if (CheckHitKey(KEY_INPUT_SPACE))
			{
				InitVariable();
				scene = PLAY;
			}
			break;

		case PLAY:	// ゲームプレイ画面
			MovePlayer();	// 自機の操作

			distance++;
			// 敵の生成
			if (timer % 120 == 0)
			{
				SetEnemy(WIDTH + 100, 500, -8, 0, 0, GetRand(IMG_ENEMY_MAX - 1));
			}
			// アイテムの生成
			if (timer % 90 == 0)
			{
				SetItem();
			}
			// 無敵時間のカウントダウン
			if (invincibleTimer > 0)
			{
				invincibleTimer--;
			}
			// 無敵時間の終了
			else
			{
				invincibleMode = false;
			}
			// トラップの生成
			if (timer % 240 == 0)
			{
				int img = GetRand(IMG_TRAP_MAX - 1);

				if (img == 0)
				{
					// 天井トラップ
					SetTrap(WIDTH + 100, 200, -8, 0, 0, img);
				}
				else
				{
					// 床トラップ
					SetTrap(WIDTH + 100, 560, -8, 0, 0, img);
				}
			}

			break;

		case OVER:	// ゲームオーバー画面
			if (timer < FPS * 3)	// 自機が爆発する演出
			{
				DrawTextC(WIDTH * 0.5, HEIGHT * 0.3, "GAME OVER", 0xff0000, 80);
			}
			if (timer > FPS * 10) scene = TITLE;	// タイトルへ遷移
			break;
			ScreenFlip();	// 裏画面の内容を表画面に反映させる
			WaitTimer(1000 / FPS);	// 1定時間待つ
			if (ProcessMessage() == -1)break;	// Windowsから情報を受け取りエラーが起きたら終了
			if (CheckHitKey(KEY_INPUT_ESCAPE) == 1)break;	// ESCキーが押されたら終了
		}
	}

	DxLib_End();	// DXライブラリ使用の終了処理
	return 0;		// ソフトの終了
}

// ここから下に自作した関数を記述する
void InitGame(void)
{
	// 背景用の画像の読み込み
	imgCastle = LoadGraphWithCheck("image/BackGround.png");
	imgFloor = LoadGraphWithCheck("image/Floor.png");
	imgCeiling = LoadGraphWithCheck("image/Ceiling2.png");
	// プレイヤー用の画像の読み込み
	imgRun = LoadGraphWithCheck("image/Run.png");
	imgJump = LoadGraphWithCheck("image/Jump.png");
	imgSliding = LoadGraphWithCheck("image/Sliding.png");
	imgAttack = LoadGraphWithCheck("image/Attack.png");
	imgDamage = LoadGraphWithCheck("image/Damage.png");
	// 敵用の画像の読み込み
	for (int i = 0; i < IMG_ENEMY_MAX; i++){
		char file[] = "image/Enemy*.png";
		file[11] = (char)('0' + i);
		imgEnemy[i] = LoadGraphWithCheck(file);
	}
	// トラップ用の画像の読み込み
	for (int i = 0; i < IMG_TRAP_MAX; i++) {
		char file[] = "image/Trap*.png";
		file[10] = (char)('0' + i);
		imgTrap[i] = LoadGraphWithCheck(file);
	}
	// その他の画像の読み込み
	imgDonut = LoadGraphWithCheck("image/Donut.png");
	imgLightning = LoadGraphWithCheck("image/Lightning.png");
	imgMagnet = LoadGraphWithCheck("image/Magnet.png");
	// サウンドの読み込みと音量設定
	bgm = LoadSoundMemWithCheck("sound/BGM.mp3");
	seDonut = LoadSoundMemWithCheck("sound/DonutSE.mp3");
	seLightning = LoadSoundMemWithCheck("sound/LightningSE.mp3");
	seMagnet = LoadSoundMemWithCheck("sound/MagnetSE.mp3");
	
	// 敵の初期化
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		enemy[i].state = 0;
	}
	// トラップの初期化
	for (int i = 0; i < TRAP_MAX; i++)
	{
		trap[i].state = 0;
	}
	// アイテムの初期化
	for (int i = 0; i < ITEM_MAX; i++)
	{
		item[i].state = 0;
	}

}
// 画像の読み込み、読み込み失敗時は通知
int LoadGraphWithCheck(const char* file) {
	int res = LoadGraph(file);
	if (res == -1) { MessageBox(GetMainWindowHandle(), file, "画像の読み込みに失敗", MB_OK | MB_ICONSTOP); }
	return res;
}

// 音声の読み込み、読み込み失敗時は通知
int LoadSoundMemWithCheck(const char* file) {
	int sou = LoadSoundMem(file);
	if (sou == -1) { MessageBox(GetMainWindowHandle(), file, "音楽の再生に失敗", MB_OK | MB_ICONSTOP); }
	return sou;
}
// 背景のスクロール
void ScrollBG(int spd)
{
	static int castleX = 0;
	static int floorX = 0;
	static int ceilingX = 0;
	// 背景
	castleX -= spd;
	if (castleX <= -WIDTH) castleX += WIDTH;
	DrawGraph(castleX, 0, imgCastle, true);
	DrawGraph(castleX + WIDTH, 0, imgCastle, true);
	// 床
	floorX += spd;
	if (floorX >= 120) floorX -= 120;
	for (int i = -1; i < 11; i++)
	{
		DrawGraph(-floorX + i * 120, 620, imgFloor, true);
	}
	// 天井
	ceilingX += spd;
	if (ceilingX >= 1200)	ceilingX -= 1200;
	for (int i = -1; i < WIDTH / 1200 + 4; i++)
	{
		DrawGraph(-ceilingX + i * 1200, -70, imgCeiling, true);
	}
}
// 敵の生成
int SetEnemy(int x, int y, int vx, int vy, int ptn, int img)
{
	for (int i = 0; i < ENEMY_MAX; i++) {
		if (enemy[i].state == 0) {
			enemy[i].x = x;
			enemy[i].y = y;
			enemy[i].vx = vx;
			enemy[i].vy = vy;
			enemy[i].state = 1;
			enemy[i].pattern = ptn;
			enemy[i].image = img;
			GetGraphSize(img, &enemy[i].wid, &enemy[i].hei);	// 画像の幅と高さを代入
			return i;
		}
	}
	return -1;
}
// 敵の移動
void MoveEnemy(void)
{
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (enemy[i].state == 0) continue;
		enemy[i].x -= 8;
		DrawGraph(enemy[i].x, enemy[i].y, imgEnemy[enemy[i].image], true);
		if (enemy[i].x < -200)
		{
			enemy[i].state = 0;
		}
	}
}
// トラップの生成
int SetTrap(int x, int y, int vx, int vy, int ptn, int img)
{
	for (int i = 0; i < TRAP_MAX; i++)
	{
		if (trap[i].state == 0)
		{
			trap[i].x = x;
			trap[i].y = y;
			trap[i].vx = vx;
			trap[i].vy = vy;
			trap[i].pattern = ptn;
			trap[i].image = img;
			trap[i].state = 1;
			return i;
		}
	}
	return -1;
}
// トラップの移動
void MoveTrap(void)
{
	for (int i = 0; i < TRAP_MAX; i++)
	{
		if (trap[i].state == 0) continue;

		trap[i].x -= 8;

		DrawGraph(trap[i].x,
			trap[i].y,
			imgTrap[trap[i].image],
			true);

		if (trap[i].x < -200)
		{
			trap[i].state = 0;
		}
	}
}
// アイテムの生成
void SetItem(void)
{
	for (int i = 0; i < ITEM_MAX; i++)
	{
		if (item[i].state == 0)
		{
			item[i].state = 1;
			item[i].x = WIDTH + 100;
			item[i].y = 250 + GetRand(250);

			item[i].image = GetRand(2);
			return;
		}
	}
}
// アイテムの移動
void MoveItem(void)
{
	for (int i = 0; i < ITEM_MAX; i++)
	{
		if (item[i].state == 0) continue;

		item[i].x -= 8;
		if (item[i].image == 0)DrawGraph(item[i].x, item[i].y, imgDonut, true);
		if (item[i].image == 1)DrawGraph(item[i].x, item[i].y, imgLightning, true);
		if (item[i].image == 2)DrawGraph(item[i].x, item[i].y, imgMagnet, true);
		if (item[i].x < -100){
			item[i].state = 0;
		}
		if (magnetMode){
			if (item[i].x < 400){
				item[i].y += (playerY - item[i].y) * 0.1f;
			}
		}
	}
}
// 衝突判定
void CheckCollision(void)
{
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (enemy[i].state == 0) continue;
		if (enemy[i].x < 180 && enemy[i].x > 0 && abs(enemy[i].y - playerY) < 120)
		{
			if (attackMode){
				enemy[i].state = 0;
				score += 500;
			}
			else if (invincibleMode){
				enemy[i].state = 0;    // 体当たりで倒す
				score += 300;
			}
			else{
				enemy[i].state = 0;
				hp -= 10;
			}
		}
	}


	for (int i = 0; i < ITEM_MAX; i++)
	{
		if (item[i].state)
		{
			if (item[i].x < 150 && item[i].x > 0 && abs(item[i].y - playerY) < 80)
			{
				if (item[i].image == 0)
				{
					score += 100;
					PlaySoundMem(seDonut, DX_PLAYTYPE_BACK);
				}
				else if (item[i].image == 1)
				{
					invincibleMode = true;
					invincibleTimer = 300;    // 約5秒
					PlaySoundMem(seLightning, DX_PLAYTYPE_BACK);
				}
				else if (item[i].image == 2)
				{
					magnetMode = true;
					magnetTimer = 300;
					PlaySoundMem(seMagnet, DX_PLAYTYPE_BACK);
				}
				item[i].state = 0;
			}
		}
	}

	if (magnetTimer > 0)
	{
		magnetTimer--;
	}
	else
	{
		magnetMode = false;
	}
}


void DrawUI(void)
{
	DrawFormatString(20, 20, GetColor(255, 255, 255), "SCORE : %d", score);
	DrawFormatString(20, 50, GetColor(255, 100, 100), "HP : %d", hp);
	DrawBox( 20, 80, 20 + hp * 2, 100, GetColor(0, 255, 0), TRUE);
	if (invincibleMode)
	{
		DrawFormatString(20, 120, GetColor(255, 255, 0), "LIGHTNING : %d", invincibleTimer / 60);
	}
	if (magnetMode)
	{
		DrawFormatString(20, 150, GetColor(0, 255, 255), "MAGNET : %d", magnetTimer / 60);
	}
}
void MovePlayer(void)
{
	bool isJump = false;
	bool attackMode = false;
	int attackTimer = 0;

	// ジャンプ
	playerVY += 0.8f;
	playerY += playerVY;

	if (playerY >= 450)
	{
		playerY = 450;
		playerVY = 250;
		isJump = false;
	}
	if (CheckHitKey(KEY_INPUT_SPACE) && !isJump) {
		DrawRectExtendGraph(0, playerVY, 0 + 150, playerVY + 200, 0, 0, playerVY, 300, imgJump, true);
	}
	// スライディング
	else if (CheckHitKey(KEY_INPUT_LSHIFT)) {
		DrawRectExtendGraph(0, 550, 0 + 230, 500 + 400, 0, 0, 393, 550, imgSliding, true);
	}
	// 攻撃
	else if (CheckHitKey(KEY_INPUT_Q)) {
		DrawRectExtendGraph(0, 500, 0 + 250, 500 + 650, 0, 0, 393, 1050, imgAttack, true);
		attackMode = true;
		attackTimer = 20;
	}
	// 走る
	else {
		for (int i = 0; i < RUN_MAX; i++)
		{
			if (timer <= 10)
			{
				DrawRectExtendGraph(0, 450, 0 + 130, 450 + 190, 30, 225, 393, 550, imgRun, true);
			}
			else if (timer <= 30 && timer >= 11)
			{
				DrawRectExtendGraph(0, 400, 0 + 130, 300 + 400, 570, 0, 393, 900, imgRun, true);
			}
			else if (timer <= 50 && timer >= 31)
			{
				DrawRectExtendGraph(0, 400, 0 + 130, 400 + 300, 1080, 0, 393, 900, imgRun, true);
			}
			else if (timer <= 70 && timer >= 51)
			{
				timer = 0;
			}
		}
	}
	// 攻撃モードのカウントダウン
	if (attackTimer > 0)
	{
		attackTimer--;
	}
	// 攻撃モードの終了
	else
	{
		attackMode = false;
	}
}

void DrawTextC(int x, int y, const char* txt, int col, int siz)
{
	SetFontSize(siz);
	int strWidth = GetDrawStringWidth(txt, strlen(txt));
	x -= strWidth / 2;
	y -= siz / 2;
	DrawString(x + 1, y + 1, txt, 0x000000);
	DrawString(x, y, txt, col);
}
void InitVariable(void)
{
	player.x = WIDTH / 2;
	player.y = HEIGHT / 2;
	player.vx = 5;
	player.vy = 5;
	GetGraphSize(imgRun, &player.wid, &player.hei);	// 画像の幅と高さを代入
	for (int i = 0; i < ENEMY_MAX; i++) enemy[i].state = 0;	// 全ての敵機を存在しない状態に
	score = 0;
	stage = 1;
}