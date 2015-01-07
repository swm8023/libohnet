#include <fn.h>
#include <stdio.h>
#include <check.h>
#include "fntest.h"



#include <vector>
using namespace std;

bool_t _op_t_copy_func(void* t1, const void* t2) {
    memcpy((void*)t1, (void*)t2, sizeof(op_t));
    return true;
}

bool_t _op_t_less_func(const void *t1, const void* t2) {
    op_t *o1 = (op_t*)t1;
    op_t *o2 = (op_t*)t2;
    return o1->type > o2->type || o1->type == o2->type && o1->num > o2->num;
}

bool_t _int_t_greater_func(const void *t1, const void *t2) {
    return *(int32_t*)t1 > *(int32_t*)t2;
}

START_TEST(test_newvector)
{
    vector_t *vec;

    vec = vector_new(int);
    ck_assert_str_eq(_CTR_CTYPE(vec)->name, "int32_t");
    vector_delete(vec);
    vec = vector_new(vector_t<int>);
    ck_assert_str_eq(_CTR_CTYPE(vec)->name, "vector_t");
    vector_delete(vec);
    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST

START_TEST(test_vectorop_cbuiltin)
{
    vector_t *vec;
    int i = 0;
    vec = vector_new(int);
    ck_assert_int_eq(vector_empty(vec), true);

    int aft_erase[] =  {-1, 1, 2, 4, 5, 6, 7, 8, 9};
    int aft_insert[] = {-1, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ck_assert_int_eq(vector_size(vec), 0);
    for (i = 0; i < 10; i++) {
        vector_push_back(vec, i);
        ck_assert_int_eq(*(int*)vector_front(vec), 0);
        ck_assert_int_eq(*(int*)vector_back(vec), i);
        ck_assert_int_eq(vector_size(vec), i + 1);
    }
    vector_update(vec, 0, -1);
    vector_erase_pos(vec, 3);
    for (i = 0; i < 9; i++) {
        ck_assert_int_eq(*(int*)vector_at(vec, i), aft_erase[i]);
    }
    vector_insert_pos(vec, 3, 3);
    for (i = 0; i < 10; i++) {
        ck_assert_int_eq(*(int*)vector_at(vec, i), aft_insert[i]);
    }

    for (i = 0; i < 10 ;i++) {
        vector_push_back(vec, i + 10);
    }
    ck_assert_int_eq(vector_size(vec), 20);
    ck_assert_int_eq(vector_empty(vec), false);
    ck_assert_int_eq(vector_capacity(vec), 32);

    vector_pop_back(vec);
    ck_assert_int_eq(vector_size(vec), 19);
    ck_assert_int_eq(*(int*)vector_back(vec), 18);

    vector_clear(vec);
    ck_assert_int_eq(vector_size(vec), 0);
    ck_assert_int_eq(vector_empty(vec), true);

    vector_delete(vec);

    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST





START_TEST(test_vectoreffec)
{
    srand((unsigned)time(NULL));
    int num = 200000, i;

    op_t *op;
    op = (op_t*)fn_malloc(num * sizeof(op_t));

    int size = 0;
    for(i = 0; i < num; i++) {
        op[i].type = i < num/10 ? 0 : rand()%4;
        switch(op[i].type) {
            // push_back
            case 0: size++, op[i].num = rand()%100; break;
            // pop_back
            case 1: size > 0 ? size-- : 0; break;
            // insert
            case 2: size++; op[i].num = rand() % size; break;
            // erase
            case 3: size > 0 ? size-- : 0; op[i].num = rand() % size; break;
        }
    }

    type_regist(op_t, _op_t_copy_func, NULL, NULL, NULL);
    // test c vector
    fntime_t delta = get_time();
    vector_t *vec = vector_new(op_t);

    for (i = 0; i < num; i++) {
        switch(op[i].type) {
            case 0: vector_push_back(vec, &op[i]); break;
            //case 1: vector_pop_back(vec); break;
            case 2: vector_insert(vec, iter_next_n(vector_begin(vec), op[i].num), &op[i]); break;
            case 3: vector_erase(vec, iter_next_n(vector_begin(vec), op[i].num)); break;
            // case 2: vector_insert_pos(vec, op[i].num, &op[i]); break;
            // case 3: vector_erase_pos(vec, op[i].num); break;
        }
    }
    delta = get_time() - delta;
    printf("C Vector Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    //test c++ vector
    delta = get_time();
    vector<op_t> svec;
    for (i = 0; i < num; i++) {
        switch(op[i].type) {
            case 0: svec.push_back(op[i]); break;
            //case 1: vector_pop_back(vec); break;
            case 2: svec.insert(svec.begin() + op[i].num, op[i]); break;
            case 3: svec.erase(svec.begin() + op[i].num); break;
        }
    }
    delta = get_time() - delta;
    printf("STL Vector Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    // check
    ck_assert_int_eq(vector_size(vec), svec.size());
    for (i = 0; i < vector_size(vec); i++) {
        op_t opp = *(op_t*)vector_at(vec, i);
        ck_assert_int_eq(opp.type, svec[i].type);
        ck_assert_int_eq(opp.num, svec[i].num);
    }

    fn_free(op);
    vector_delete(vec);
    
    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);

    
}
END_TEST

START_TEST(test_vector_iterator)
{
    vector_t *vec;
    int i = 0;
    vec = vector_new(int);
    ck_assert_int_eq(vector_empty(vec), true);

    for (i = 0;i < 10; i++) {
        vector_push_back(vec, i);
    }

    int aft_erase[] =  {-1, 1, 2, 4, 5, 6, 7, 8, 9};
    int aft_insert[] = {-1, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    iterator_t it0, it1, it2;
    int val1, val2;
    _ITER_INIT(it1);

    ck_assert_int_eq(_iter_is_vaild(it1), false);
    it1 = vector_begin(vec);
    ck_assert_int_eq(_iter_is_vaild(it1), true);
    iter_get_value(it1, &val1);
    ck_assert_int_eq(val1, 0);

    it2 = vector_end(vec);
    ck_assert_int_eq(iter_distance(it1, it2), 10);
    ck_assert_int_eq(iter_distance(it2, it1), -10);

    it0 = vector_begin(vec);
    ck_assert_int_eq(iter_equal(it0, it1), true);
    ck_assert_int_eq(iter_equal(it0, it2), false);

    int ind = 0, value;
    for (it0 = vector_begin(vec); !iter_equal(it0, vector_end(vec));
        it0 = iter_next(it0)) {
        iter_get_value(it0, &value);
        ck_assert_int_eq(value, ind);
        ck_assert_int_eq(*(int*)iter_get_pointer(it0), ind);
        ind ++;
    }

    it0 = iter_next_n(vector_begin(vec), 5);
    iter_get_value(it0, &value);
    ck_assert_int_eq(value, 5);

    it0 = iter_next_n(it0, -3);
    iter_get_value(it0, &value);
    ck_assert_int_eq(value, 2);

    it0 = iter_pre_n(it0, -5);
    iter_get_value(it0, &value);
    ck_assert_int_eq(value, 7);

    it0 = iter_pre_n(it0, 4);
    iter_get_value(it0, &value);
    ck_assert_int_eq(value, 3);

    value = -1;
    iter_set_value(vector_begin(vec), &value);
    ck_assert_int_eq(*(int*)vector_at(vec, 0), -1);

    vector_erase(vec, iter_next_n(vector_begin(vec), 3));
    for (i = 0; i < 9; i++) {
        ck_assert_int_eq(*(int*)vector_at(vec, i), aft_erase[i]);
    }
    vector_insert(vec, iter_next_n(vector_begin(vec), 3), 3);
    for (i = 0; i < 10; i++) {
        ck_assert_int_eq(*(int*)vector_at(vec, i), aft_insert[i]);
    }

    vector_delete(vec);

    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST


START_TEST(test_vector_swapelem) {
    int i;
    vector_t *vec;
    vec = vector_new(int);
    int arr0[] = {1, 2, 3, 4 , 5, 6, 7, 8, 9, 10};
    int arr1[] = {1, 2, 5, 4, 3, 6, 7, 10, 9, 8};
    for (i = 0; i < 10; i++) vector_push_back(vec, arr0[i]);
    _vector_swap_elem(vec, 2, 4);
    _vector_swap_elem(vec, 9, 7);
    _vector_swap_elem(vec, 3, 3);
    for (i = 0; i < 10; i++) {
        ck_assert_int_eq(arr1[i], *(int*)vector_at(vec, i));
    }
    vector_delete(vec);


    type_regist(op_t, _op_t_copy_func, NULL, NULL, NULL);
    vec = vector_new(op_t);
    op_t op0[] = {{1, 2}, {3, 4}, {5, 6}};
    op_t op1[] = {{5, 6}, {3, 4}, {1, 2}};
    for (i = 0;i < 3; i++) {
        vector_push_back(vec, &op0[i]);
    }
    _vector_swap_elem(vec, 0, 2);
    _vector_swap_elem(vec, 1, 1);
    for (i = 0;i < 3; i++) {
        op_t *op = (op_t*)vector_at(vec, i);
        ck_assert_int_eq(op->type, op1[i].type);
        ck_assert_int_eq(op->num, op1[i].num);
    }
    vector_delete(vec);

    type_unregist(op_t);
    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST

Suite *cvector_test_suite() {
	Suite *s = suite_create("=== CVector Test ===");
	TCase *tc_core = tcase_create("TC core");
	tcase_set_timeout(tc_core, 1000);

	tcase_add_test(tc_core, test_newvector);
    tcase_add_test(tc_core, test_vectorop_cbuiltin);
    tcase_add_test(tc_core, test_vector_iterator);
    tcase_add_test(tc_core, test_vector_swapelem);
#ifdef ENEFFTEST
    tcase_add_test(tc_core, test_vectoreffec);
#endif

	suite_add_tcase(s, tc_core);
	return s;
}