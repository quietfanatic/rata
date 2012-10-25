


struct Test_Area : Serialized<Test_Area, "Test_Area">, Physical {
    Test_Area () {
        
    }
    void serialize (Serializer* s) {
        serialize_body(s);
    }
};






