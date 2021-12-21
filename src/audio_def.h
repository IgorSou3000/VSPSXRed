#define XA_LENGTH(x) (((u64)(x) * 75) / 100 * IO_SECT_SIZE) //Centiseconds to sectors in bytes (w)

typedef struct
{
	XA_File file;
	u32 length;
} XA_TrackDef;

static const XA_TrackDef xa_tracks[] = {
	//MENU.XA
	{XA_Menu, XA_LENGTH(11400)}, //XA_GettinFreaky
	{XA_Menu, XA_LENGTH(3840)},  //XA_GameOver
	//WEEK1A.XA
	{XA_Week1A, XA_LENGTH(13500)}, //XA_Bopeebo
	{XA_Week1A, XA_LENGTH(25100)}, //XA_Fresh
};

static const char *xa_paths[] = {
	"\\MUSIC\\MENU.XA;1",   //XA_Menu
	"\\MUSIC\\WEEK1A.XA;1", //XA_Week1A
	NULL,
};

typedef struct
{
	const char *name;
	boolean vocal;
} XA_Mp3;

static const XA_Mp3 xa_mp3s[] = {
	//MENU.XA
	{"freaky", false},   //XA_GettinFreaky
	{"gameover", false}, //XA_GameOver
	//WEEK1A.XA
	{"bopeebo", true}, //XA_Bopeebo
	{"fresh", false},   //XA_Fresh
	//WEEK1B.XA
	{"dadbattle", true}, //XA_Dadbattle
	{"tutorial", false}, //XA_Tutorial
	//WEEK2A.XA
	{"spookeez", true}, //XA_Spookeez
	{"south", true},    //XA_South
	//WEEK2B.XA
	{"monster", true}, //XA_Monster
	{"clucked", true}, //XA_Clucked
	//WEEK3A.XA
	{"pico", true},   //XA_Pico
	{"philly", true}, //XA_Philly
	//WEEK3B.XA
	{"blammed", true}, //XA_Blammed
	//WEEK4A.XA
	{"satinpanties", true}, //XA_SatinPanties
	{"high", true},         //XA_High
	//WEEK4B.XA
	{"milf", true}, //XA_MILF
	{"test", true}, //XA_Test
	//WEEK5A.XA
	{"cocoa", true},  //XA_Cocoa
	{"eggnog", true}, //XA_Eggnog
	//WEEK5B.XA
	{"winterhorrorland", true}, //XA_WinterHorrorland
	//WEEK6A.XA
	{"senpai", true}, //XA_Senpai
	{"roses", true},  //XA_Roses
	//WEEK6B.XA
	{"thorns", true}, //XA_Thorns
	//WEEK7A.XA
	{"ugh", true},  //XA_Ugh
	{"guns", true}, //XA_Guns
	//WEEK7B.XA
	{"stress", true}, //XA_Stress
	//KAPIA.XA
	{"wocky", true},     //XA_Wocky
	{"beathoven", true}, //XA_Beathoven
	//KAPIB.XA
	{"hairball", true}, //XA_Hairball
	{"nyaw", true},     //XA_Nyaw
	//CLWNA.XA
	{"improbableoutset", true}, //XA_ImprobableOutset
	{"madness", true}, //XA_Madness
	//CLWNB.XA
	{"hellclown", true}, //XA_Hellclown
	{"expurgation", true}, //XA_Expurgation
	
	{NULL, false}
};
