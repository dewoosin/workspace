/**
 * @file ble_manager.h
 * @brief BLE 통신 관리 모듈
 * @version 1.0
 * @date 2024-12-28
 * 
 * 이 모듈은 ESP32의 BLE 서버 기능을 관리합니다.
 * 클라이언트 연결, 데이터 수신, 응답 전송을 담당합니다.
 */

#pragma once

#include <NimBLEDevice.h>
#include <Arduino.h>
#include "config.h"

/**
 * @brief BLE 연결 상태 열거형
 */
enum BLEConnectionState {
    BLE_STATE_DISCONNECTED = 0,   ///< 연결 해제됨
    BLE_STATE_ADVERTISING,        ///< 광고 중
    BLE_STATE_CONNECTED,          ///< 연결됨
    BLE_STATE_ERROR              ///< 오류 상태
};

/**
 * @brief BLE 수신 데이터 구조체
 */
struct BLEReceivedData {
    char* data;                   ///< 수신된 데이터
    size_t length;                ///< 데이터 길이
    uint32_t timestamp;           ///< 수신 시간
    bool valid;                   ///< 데이터 유효성
};

/**
 * @brief BLE 관리자 클래스
 * 
 * ESP32의 BLE 서버 기능을 추상화하고 관리합니다.
 * 안전한 연결 관리와 데이터 전송을 제공합니다.
 */
class BLEManager {
public:
    /**
     * @brief BLE 시스템 초기화
     * @return true 성공, false 실패
     * 
     * NimBLE 스택을 초기화하고 GHOSTYPE 서비스를 생성합니다.
     * 광고를 시작하여 클라이언트 연결을 대기합니다.
     */
    static bool initialize();

    /**
     * @brief BLE 시스템 종료
     * 
     * 모든 연결을 해제하고 BLE 스택을 정리합니다.
     */
    static void deinitialize();

    /**
     * @brief 수신 데이터 확인
     * @return true 데이터 있음, false 없음
     * 
     * BLE 클라이언트로부터 수신된 데이터가 있는지 확인합니다.
     */
    static bool hasReceivedData();

    /**
     * @brief 수신 데이터 가져오기
     * @return 수신된 데이터 구조체
     * 
     * 수신된 데이터를 가져옵니다. 가져온 후에는 내부 버퍼가 비워집니다.
     */
    static BLEReceivedData getReceivedData();

    /**
     * @brief 응답 데이터 전송
     * @param response 전송할 응답 데이터
     * @return true 성공, false 실패
     * 
     * BLE 클라이언트에게 응답을 전송합니다.
     * 연결된 클라이언트가 있을 때만 전송됩니다.
     */
    static bool sendResponse(const String& response);

    /**
     * @brief 현재 연결 상태 조회
     * @return 현재 BLE 연결 상태
     * 
     * BLE 서버의 현재 연결 상태를 반환합니다.
     */
    static BLEConnectionState getConnectionState();

    /**
     * @brief 연결된 클라이언트 수 조회
     * @return 연결된 클라이언트 수
     * 
     * 현재 연결된 BLE 클라이언트의 수를 반환합니다.
     */
    static uint8_t getConnectedClientCount();

    /**
     * @brief 광고 재시작
     * @return true 성공, false 실패
     * 
     * BLE 광고를 중단했다가 다시 시작합니다.
     * 연결 문제 해결에 사용됩니다.
     */
    static bool restartAdvertising();

    /**
     * @brief 연결 매개변수 업데이트
     * @param min_interval 최소 연결 간격
     * @param max_interval 최대 연결 간격
     * @param latency 연결 지연
     * @param timeout 연결 타임아웃
     * @return true 성공, false 실패
     * 
     * BLE 연결의 매개변수를 업데이트합니다.
     */
    static bool updateConnectionParams(uint16_t min_interval, uint16_t max_interval, 
                                     uint16_t latency, uint16_t timeout);

    /**
     * @brief 클라이언트 연결 해제
     * @return true 성공, false 실패
     * 
     * 현재 연결된 모든 클라이언트와의 연결을 해제합니다.
     */
    static bool disconnectAllClients();

    /**
     * @brief BLE 시스템 상태 확인
     * @return true 정상, false 비정상
     * 
     * BLE 시스템의 전반적인 상태를 확인합니다.
     */
    static bool isSystemHealthy();

    /**
     * @brief 통계 정보 조회
     * @param[out] bytes_received 총 수신 바이트
     * @param[out] bytes_sent 총 송신 바이트
     * @param[out] connection_count 총 연결 횟수
     * 
     * BLE 통신 통계 정보를 조회합니다.
     */
    static void getStatistics(uint32_t& bytes_received, uint32_t& bytes_sent, uint32_t& connection_count);

private:
    // NimBLE 객체들
    static NimBLEServer* ble_server;              ///< BLE 서버 인스턴스
    static NimBLEService* ble_service;            ///< GHOSTYPE 서비스
    static NimBLECharacteristic* char_rx;         ///< 수신 특성 (클라이언트 → 서버)
    static NimBLECharacteristic* char_tx;         ///< 송신 특성 (서버 → 클라이언트)
    static NimBLEAdvertising* ble_advertising;    ///< 광고 관리자

    // 상태 관리
    static bool initialized;                      ///< 초기화 상태
    static BLEConnectionState connection_state;   ///< 현재 연결 상태
    static uint8_t connected_clients;             ///< 연결된 클라이언트 수

    // 데이터 버퍼
    static char* receive_buffer;                  ///< 수신 데이터 버퍼
    static size_t receive_buffer_size;            ///< 수신 버퍼 크기
    static size_t received_data_length;           ///< 수신된 데이터 길이
    static uint32_t last_receive_time;            ///< 마지막 수신 시간

    // 통계 정보
    static uint32_t total_bytes_received;         ///< 총 수신 바이트
    static uint32_t total_bytes_sent;             ///< 총 송신 바이트
    static uint32_t total_connections;            ///< 총 연결 횟수

    /**
     * @brief 수신 버퍼 초기화
     * @return true 성공, false 실패
     */
    static bool initializeReceiveBuffer();

    /**
     * @brief 수신 버퍼 정리
     */
    static void cleanupReceiveBuffer();

    /**
     * @brief 수신 데이터를 버퍼에 저장
     * @param data 수신된 데이터
     * @param length 데이터 길이
     * @return true 성공, false 실패
     */
    static bool storeReceivedData(const uint8_t* data, size_t length);

    /**
     * @brief 연결 상태 업데이트
     * @param new_state 새로운 연결 상태
     */
    static void updateConnectionState(BLEConnectionState new_state);

    /**
     * @brief 통계 정보 업데이트
     * @param bytes_received 수신된 바이트 수
     * @param bytes_sent 송신된 바이트 수
     */
    static void updateStatistics(size_t bytes_received, size_t bytes_sent);

    // 콜백 클래스들
    class ServerCallbacks;
    class CharacteristicCallbacks;
};