#include "DxLib.h"
#include "Run.h"

const int WIDTH = 1200, HEIGHT = 720;	// ウィンドウの幅と高さのピクセル数
const int FPS = 60;	// フレームレート
const int IMG_ENEMY_MAX = 4;	// 敵の画像の枚数

// グローバル変数
// ここでゲームに用いる変数や配列を定義する
int imgCastle, imgFloor, ;	// 背景画像
int imgRun, imgJump, imgSliding, imgAttack, imgDamage;		// プレイヤーの画像
int imgEnemy[IMG_ENEMY_MAX];	// 敵の画像
int imgDonut, intLightning, intMagnet;	// アイテムの画像
int bgm,  seDonut, seLightning, seMagnet;		// 音の読み込み用
int distance = 0;	// ステージ終端までの距離
int stage = 1;	// ステージ

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

		// ゲームの骨組みとなる処理を、ここに記入する
		DrawGraph(0, 0, imgCastle, false);	// 背景画像を描画する

		ScreenFlip();	// 裏画面の内容を表画面に反映させる
		WaitTimer(1000 / FPS);	// 1定時間待つ
		if (ProcessMessage() == -1)break;	// Windowsから情報を受け取りエラーが起きたら終了
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1)break;	// ESCキーが押されたら終了
	}

	DxLib_End();	// DXライブラリ使用の終了処理
	return 0;		// ソフトの終了
}

// ここから下に自作した関数を記述する
void InitGame(void)
{
	// 背景用の画像の読み込み
	imgCastle = LoadGraphWithCheck("image/BackGround.png");
	imgFloor = LoadGraphWithCheck("image/Floor.psd");
	// プレイヤー用の画像の読み込み
	imgRun = LoadGraphWithCheck("image/Run.psd");
	imgJump = LoadGraphWithCheck("image/Jump.psd");
	imgSliding = LoadGraphWithCheck("image/Sliding.psd");
	imgAttack = LoadGraphWithCheck("image/Attack.psd");
	imgDamage = LoadGraphWithCheck("image/Damage.psd");
	// 敵用の画像の読み込み
	for (int i = 0; i < IMG_ENEMY_MAX; i++){
		char file[] = "image/Enemy*.psd";
		file[11] = (char)('0' + i);
		imgEnemy[i] = LoadGraphWithCheck(file);
	}
	// その他の画像の読み込み
	imgDonut = LoadGraphWithCheck("image/Donut.psd");
	intLightning = LoadGraphWithCheck("image/Lightning.psd");
	intMagnet = LoadGraphWithCheck("image/Magnet.psd");

	// サウンドの読み込みと音量設定
	bgm = LoadSoundMemWithCheck("sound/BGM.mp3");
	seDonut = LoadSoundMemWithCheck("sound/DonutSE.mp3");
	seLightning = LoadSoundMemWithCheck("sound/LightningSE.mp3");
	seMagnet = LoadSoundMemWithCheck("sound/MagnetSE.mp3");
	ChangeVolumeSoundMem(128, bgm);
}
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