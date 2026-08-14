#include "AudioManager.h"

AudioManager::AudioManager() : dfSerial(2), isDfPlayerReady(false) {}

void AudioManager::begin() {
    // تهيئة الـ Buzzer
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
    
    // تهيئة السيريال للـ DFPlayer على UART2
    dfSerial.begin(9600, SERIAL_8N1, PIN_DFPLAYER_RX, PIN_DFPLAYER_TX);
    
    if (dfPlayer.begin(dfSerial)) {
        isDfPlayerReady = true;
        dfPlayer.volume(20);
    } else {
        isDfPlayerReady = false;
    }
}

// الدالة القديمة المباشرة للتنبيه النجاح/الجرعة
void AudioManager::playNotification() {
    if (isDfPlayerReady) {
        dfPlayer.play(1); // تشغيل المقطع رقم 1 (تنبيه الجرعة)
    } else {
        playBuzzerBeep(150);
    }
}

// الدالة القديمة المباشرة لنغمة الخطأ (متوافقة مع FreeRTOS)
void AudioManager::playErrorTone() {
    for (int i = 0; i < 3; i++) {
        digitalWrite(PIN_BUZZER, HIGH);
        vTaskDelay(pdMS_TO_TICKS(80)); // Non-blocking delay for FreeRTOS
        digitalWrite(PIN_BUZZER, LOW);
        vTaskDelay(pdMS_TO_TICKS(80));
    }
}

void AudioManager::playBuzzerBeep(uint16_t durationMs) {
    digitalWrite(PIN_BUZZER, HIGH);
    vTaskDelay(pdMS_TO_TICKS(durationMs));
    digitalWrite(PIN_BUZZER, LOW);
}

void AudioManager::playTrack(uint8_t trackNumber) {
    if (isDfPlayerReady) {
        dfPlayer.play(trackNumber);
    } else {
        playBuzzerBeep(200);
    }
}

void AudioManager::setVolume(uint8_t volume) {
    if (isDfPlayerReady && volume <= 30) {
        dfPlayer.volume(volume);
    }
}

void AudioManager::stopAudio() {
    if (isDfPlayerReady) {
        dfPlayer.stop();
    }
}