#if 0
    gx = _discard_gyro_value(gx);
    gy = _discard_gyro_value(gy);
    gz = _discard_gyro_value(gz);

    // measure::begin(); // 23
    float dx = gyro_raw2degree((ax < 0 ? -gx : gx), interval);
    float dy = gyro_raw2degree((ay < 0 ? -gy : gy), interval);
    float dz = gyro_raw2degree((az < 0 ? -gz : gz), interval);

    int32_t ax2 = ax * ax, ay2 = ay * ay, az2 = az * az;
    double n = ay2 + az2, m = n + ax2;

    float yaw   = float((ax2/m * dx) + (ay2/m * dy) + (az2/m * dz));
    float pitch = float((az2/n * dy) + (ay2/n * dz));
#endif