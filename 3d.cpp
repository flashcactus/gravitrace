quaternion rot2q(double angle, quaternion axis){
    quaternion q = axis;
    q.r = 0;
    q /= q.abs();
    q *= sin(angle/2);
    q.r = cos(angle/2);
    return q;
}

Rotation::Rotation(double yaw, double pitch, double roll){ //tait-bryan (intrinsic) angles
    quaternion X(0,1,0,0);
    quaternion Y(0,0,1,0);
    quaternion Z(0,0,0,1);
    quaternion yaw_rot = rot2q(yaw, Z);
    quaternion pitch_rot = rot2q(pitch, Y);
    quaternion roll_rot = rot2q(roll, X);
    rot_quaternion = roll_rot*pitch_rot*yaw_rot;
}

Rotation::Rotation(double phi, vec3 axis){ //axis-angle representation
    quaternion ax_q(0,axis.x,axis.y,axis.z);
    rot_quaternion = rot2q(phi,ax_q);
}

Rotation::Rotation(quaternion q){
    rot_quaternion = q;    
}

Rotation::Rotation(){ 
    rot_quaternion = quaternion(1,0,0,0);
}

template<typename V> V Rotation::rotate(V v) {
    quaternion q(0,v.x,v.y,v.z);
    q = rot_quaternion.conjugate() * q * rot_quaternion;
    return V(q.i,q.j,q.k);
}

/*Rotation& operator*=(const &Rotation other){
    rot_quaternion = other.rot_quaternion * rot_quaternion;
    return *this;
}*/

Photon Camera::emit_photon(int img_x, int img_y, double speed_of_light) {
    vec3 velocity;
    //set direction for fixed-size, variable-distance screen
    velocity.x = resolution_h / tan(FOV/2);
    velocity.y = resolution_h - 2 * img_y;
    velocity.z = resolution_v - 2 * img_x;
    //scale it appropriately
    velocity = normalize(velocity,speed_of_light);
    return Photon(pos, rot.rotate(velocity));
}

