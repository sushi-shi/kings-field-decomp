ADDRESS_AT("GAME", 0x80014d34, 0xd4)
ADDRESS_AT("OPEN", 0x80015b0c, 0xd4)
void pitch_yaw_to_forward_vector(const struct KfEulerAngles *angles, SVECTOR *direction)
{
    MATRIX pitch_matrix;
    MATRIX yaw_matrix;
    SVECTOR source;
    VECTOR result;

    setVector(&source, 0, 0, KF_FIXED12_ONE);
    matrix_set_rotation_x(-angles->x & KF_ANGLE_WRAP_MASK, &pitch_matrix);
    ApplyMatrix(&pitch_matrix, &source, &result);
    copyVector(&source, &result);
    matrix_set_rotation_y(angles->y, &yaw_matrix);
    ApplyMatrix(&yaw_matrix, &source, &result);
    copyVector(direction, &result);
}
