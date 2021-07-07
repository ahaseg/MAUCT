// Spresense with 改造PIRセンサ
// センサでの信号取得＋ランダム音楽再生
// MAUCTにファイル名を改称
// 210701

// Music
#include <SDHCI.h>
#include <Audio.h>
SDClass theSD;
AudioClass *theAudio;
File myFile;
bool ErrEnd = false;
static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}

err_t err;

// PIR
#define PIR_PIN 16
int pirValue;
bool bDetect = false;
void detect() {
  bDetect = true;
}

void setup() {
  // Debug
  Serial.begin(115200);
  Serial.println("PIR and sound:");

  /* Initialize SD */
  while (!theSD.begin())
    {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }

  // start audio system
  theAudio = AudioClass::getInstance();
  theAudio->begin(audio_attention_cb);
  puts("initialization Audio Library");

  /* Set clock mode to normal */
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);
  err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);

  /* Verify player initialize */
  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("Player0 initialize error\n");
      exit(1);
    }

  // PIR Monitor
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), detect, RISING);
  digitalWrite(LED0, LOW);  

  // PIR Initialize
    Serial.println("Wait for 30 sec.");
    for (int i=0; i < 30; i++) {
      delay(600);
      Serial.print(i+" ");
    } 
    Serial.println("Initialize DONE");
}

void loop() {  
  if (bDetect) {
    digitalWrite(LED0, HIGH);
    Serial.println("DETECT!");

    puts("Play!");

    /* Open file placed on SD card */
    long randNumber = random(5);
    if (randNumber == 0) myFile = theSD.open("/audio/sound1.mp3"); 
    if (randNumber == 1) myFile = theSD.open("/audio/sound2.mp3"); 
    if (randNumber == 2) myFile = theSD.open("/audio/sound3.mp3"); 
    if (randNumber == 3) myFile = theSD.open("/audio/sound4.mp3"); 
    if (randNumber == 4) myFile = theSD.open("/audio/sound5.mp3"); 

    /* Verify file open */
    if (!myFile)
    {
      printf("File open error\n");
      exit(1);
    }
    printf("Open! %d\n",myFile);

    /* Send first frames to be decoded */
    err = theAudio->writeFrames(AudioClass::Player0, myFile);
    if ((err != AUDIOLIB_ECODE_OK) && (err != AUDIOLIB_ECODE_FILEEND))
    {
      printf("File Read Error! =%d\n",err);
      myFile.close();
      return;
     }

      /* Main volume set to -16.0 dB */
     theAudio->setVolume(-160);
     theAudio->startPlayer(AudioClass::Player0);
    
      bDetect = false;
      delay(300);
      digitalWrite(LED0, LOW);

      /*  Tell when player file ends */
      if (err == AUDIOLIB_ECODE_FILEEND)
      {
        printf("Main player File End!\n");
        theAudio->stopPlayer(AudioClass::Player0, AS_STOPPLAYER_ESEND);
        myFile.close();
        err = 0;
        return;
      }
      /* Show error code from player and stop */
      if (err)
      {
        printf("Main player error code: %d\n", err);
      goto stop_player;
      }

      if (ErrEnd)
      {
        printf("Error End\n");
        goto stop_player;
      }

  return;

  stop_player:
    theAudio->stopPlayer(AudioClass::Player0);
    myFile.close();
    theAudio->setReadyMode();
    theAudio->end();
    exit(1);
  } else {
    digitalWrite(LED0, LOW);
  }
}
