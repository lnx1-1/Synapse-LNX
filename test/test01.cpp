#include <Arduino.h>
#include <unity.h>

// Beispiel-Funktion zum Testen
int add(int a, int b) {
    return a + b;
}

void test_add() {
    TEST_ASSERT_EQUAL(5, add(2, 3));
    TEST_ASSERT_EQUAL(0, add(-2, 2));
    TEST_ASSERT_EQUAL(-5, add(-2, -3));
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_add);
    UNITY_END();
}

void loop() {
    // Nicht benötigt für Unit-
}