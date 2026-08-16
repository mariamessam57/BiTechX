#include "AudioManager.h"

// استخدام منفذ Serial 2 المدمج في الـ ESP32
AudioManager::AudioManager(uint8_t rx, uint8_t tx, uint8_t busy)
    : audioSerial(2), rxPin(rx), txPin(tx), busyPin(busy), audioMutex(NULL), isInitialized(false) {}

bool AudioManager::begin(uint8_t initialVolume) {
    if (audioMutex == NULL) {
        audioMutex = xSemaphoreCreateMutex();
    }

    if (busyPin != 255) {
        // تفعيل المقاومة الداخلية لمنع القراءات العائمة
        pinMode(busyPin, INPUT_PULLUP);
    }

    // تهيئة الـ UART بمعدل 9600 Baud الخاص بـ DFPlayer
    audioSerial.begin(9600, SERIAL_8N1, rxPin, txPin);
    vTaskDelay(pdMS_TO_TICKS(500)); // مهلة لاستقرار اتصال السيريال

    if (!dfPlayer.begin(audioSerial)) {
        isInitialized = false;
        return false;
    }

    isInitialized = true;
    setVolume(initialVolume);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    return true;
}

void AudioManager::setVolume(uint8_t volume) {
    if (!isInitialized) return;
    
    if (audioMutex != NULL && xSemaphoreTake(audioMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        volume = constrain(volume, 0, 30);
        dfPlayer.volume(volume);
        vTaskDelay(pdMS_TO_TICKS(20)); // مهلة بسيطة لمعالجة الأمر في DFPlayer
        xSemaphoreGive(audioMutex);
    }
}

void AudioManager::playTrack(SoundTrack track) {
    playTrackNumber(static_cast<uint16_t>(track));
}

void AudioManager::playTrackNumber(uint16_t trackNumber) {
    if (!isInitialized) return;

    if (audioMutex != NULL && xSemaphoreTake(audioMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        dfPlayer.play(trackNumber);
        vTaskDelay(pdMS_TO_TICKS(50)); // مهلة لبدء قراءة الملف الصوتي
        xSemaphoreGive(audioMutex);
    }
}

void AudioManager::stop() {
    if (!isInitialized) return;

    if (audioMutex != NULL && xSemaphoreTake(audioMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        dfPlayer.stop();
        vTaskDelay(pdMS_TO_TICKS(20));
        xSemaphoreGive(audioMutex);
    }
}

void AudioManager::pause() {
    if (!isInitialized) return;

    if (audioMutex != NULL && xSemaphoreTake(audioMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        dfPlayer.pause();
        vTaskDelay(pdMS_TO_TICKS(20));
        xSemaphoreGive(audioMutex);
    }
}

void AudioManager::resume() {
    if (!isInitialized) return;

    if (audioMutex != NULL && xSemaphoreTake(audioMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        dfPlayer.start();
        vTaskDelay(pdMS_TO_TICKS(20));
        xSemaphoreGive(audioMutex);
    }
}

void AudioManager::triggerMedicineAlarm() {
    // تشغيل صوت التنبيه الخاص بموعد تناول الدواء
    playTrack(SoundTrack::MEDICINE_REMINDER);
}

bool AudioManager::isPlaying() {
    if (!isInitialized) return false;

    // الطريقة الأولى والأسرع: فحص بنة الـ BUSY المباشرة
    if (busyPin != 255) {
        return digitalRead(busyPin) == LOW;
    }

    // الطريقة البديلة: الاستعلام عبر أوامر السيريال بحماية الـ Mutex
    bool active = false;
    if (audioMutex != NULL && xSemaphoreTake(audioMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        active = (dfPlayer.readState() == 1);
        xSemaphoreGive(audioMutex);
    }
    return active;
}