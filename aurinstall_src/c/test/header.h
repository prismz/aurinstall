struct example_struct {
    int value;
};

void modify_val(struct example_struct* s, int val) {
    s->value = val;
}