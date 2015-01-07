#include <fn.h>
#include <stdio.h>
#include <check.h>

#include "fntest.h"

#include <vector>
#include <list>
using namespace std;


START_TEST(test_newlist)
{
    list_t *lst;

    lst = list_new(int);
    ck_assert_str_eq(_CTR_CTYPE(lst)->name, "int32_t");
    list_delete(lst);
    lst = list_new(vector_t<int>);
    ck_assert_str_eq(_CTR_CTYPE(lst)->name, "vector_t");
    list_delete(lst);
    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST

START_TEST(test_listop_cbuiltin)
{
    list_t *lst;
    int i = 0;
    lst = list_new(int);
    ck_assert_int_eq(list_empty(lst), true);
    ck_assert_int_eq(list_size(lst), 0);


    for (i = 0; i < 10; i++) {
        list_push_back(lst, i);
        ck_assert_int_eq(*(int*)list_front(lst), 0);
        ck_assert_int_eq(*(int*)list_back(lst), i);
        ck_assert_int_eq(list_size(lst), i + 1);
    }

    for (i = 0; i < 10; i++) {
        list_push_front(lst, i);
        ck_assert_int_eq(*(int*)list_front(lst), i);
        ck_assert_int_eq(*(int*)list_back(lst), 9);
        ck_assert_int_eq(list_size(lst), i + 11);
    }

    int *p, ind = 0, nums[] = {9,8,7,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,8,9}, val;
    iterator_t it0;
    it0 = list_begin(lst);


    for (it0 = list_begin(lst); !iter_equal(it0, list_end(lst)); it0 = iter_next(it0)) {
        iter_get_value(it0, &val);
        ck_assert_int_eq(nums[ind++], val);
        ck_assert_int_eq(*(int*)iter_get_pointer(it0), val);
    }

    for (i = 0; i < 10; i++) {
        ck_assert_int_eq(*(int*)list_front(lst), 9-i);
        ck_assert_int_eq(*(int*)list_back(lst), 9-i);
        ck_assert_int_eq(list_size(lst), 20 - 2*i);
        list_pop_front(lst);
        list_pop_back(lst);
    }
    ck_assert_int_eq(list_empty(lst), true);
    ck_assert_int_eq(list_size(lst), 0);

    list_delete(lst);

    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST

START_TEST(test_listop_userdef)
{
    type_regist(op_t, _op_t_copy_func, NULL, NULL, NULL);
    list_t *lst;
    int i = 0;
    lst = list_new(op_t);
    iterator_t it;
    ck_assert_int_eq(list_empty(lst), true);
    ck_assert_int_eq(list_size(lst), 0);

    for (i = 0; i < 10; i++) {
        op_t op = {i, i + 1};
        list_push_back(lst, &op);
        ck_assert_int_eq(((op_t*)list_front(lst))->type, 0);
        ck_assert_int_eq(((op_t*)list_back(lst))->type, i);
        ck_assert_int_eq(((op_t*)list_front(lst))->num, 1);
        ck_assert_int_eq(((op_t*)list_back(lst))->num, i + 1);
        ck_assert_int_eq(list_size(lst), i + 1);
    }

    for (i = 0; i < 10; i++) {
        op_t op = {i, i +1};
        list_push_front(lst, &op);
        ck_assert_int_eq(((op_t*)list_front(lst))->type, i);
        ck_assert_int_eq(((op_t*)list_back(lst))->type, 9);
        ck_assert_int_eq(((op_t*)list_front(lst))->num, i +1);
        ck_assert_int_eq(((op_t*)list_back(lst))->num, 10);
        ck_assert_int_eq(list_size(lst), i + 11);
    }

    op_t p;
    int ind = 0, nums[] = {9,8,7,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,8,9};
    for(it = list_begin(lst); !iter_equal(it, list_end(lst)); it = iter_next(it)){
        iter_get_value(it, &p);
        ck_assert_int_eq(nums[ind], p.type);
        ck_assert_int_eq(nums[ind++], p.num - 1);
    }

    for (i = 0; i < 10; i++) {
        ck_assert_int_eq(((op_t*)list_front(lst))->type, 9-i);
        ck_assert_int_eq(((op_t*)list_back(lst))->type, 9-i);
        ck_assert_int_eq(((op_t*)list_front(lst))->num, 10-i);
        ck_assert_int_eq(((op_t*)list_back(lst))->num, 10-i);
        ck_assert_int_eq(list_size(lst), 20 - 2*i);
        list_pop_front(lst);
        list_pop_back(lst);
    }
    ck_assert_int_eq(list_empty(lst), true);
    ck_assert_int_eq(list_size(lst), 0);

    list_delete(lst);


    list_t *lstp = list_new(op_t*);

    for (i = 0; i < 10; i++) {
        op_t *op = (op_t*)fn_malloc(sizeof(op_t));
        op->type = i;
        op->num = i + 1;
        list_push_back(lst, op);
        op_t *fop = *(op_t**)list_front(lst);
        op_t *bop = *(op_t**)list_back(lst);
        ck_assert_int_eq(fop->type, 0);
        ck_assert_int_eq(bop->type, i);
        ck_assert_int_eq(fop->num, 1);
        ck_assert_int_eq(bop->num, i + 1);
        ck_assert_int_eq(list_size(lst), i + 1);
    }

    while(!list_empty(lstp)) {
        op_t *p = *(op_t**)iter_get_pointer(list_begin(lstp));
        fn_free(p);
        list_pop_front(lstp);
    }


    list_delete(lstp);

    type_unregist(op_t);
    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);

}
END_TEST



START_TEST(test_listeffec)
{
    srand((unsigned)time(NULL));
    int num = 100000, i;

    op_t *op;
    op = (op_t*)fn_malloc(num * sizeof(op_t));

    int size = 0;
    for(i = 0; i < num; i++) {
        op[i].type = i < num/10 ? 0 : rand()%6;
        switch(op[i].type) {
            // push_front
            case 0: size++, op[i].num = rand()%100; break;
            // push_back
            case 1: size++, op[i].num = rand()%100; break;
            // pop_front
            case 2: if (size > 0)  size-- ; break;
            // pop_back
            case 3: if (size > 0)  size-- ; break;
            // insert
            case 4: size++; op[i].num = rand() % size; break;
            // erase
            case 5: if (size > 0) size--, op[i].num = rand() % size; break;
        }
    }

    type_regist(op_t, _op_t_copy_func, NULL, NULL, NULL);
    // test c list
    fntime_t delta = get_time();
    list_t *lst = list_new(op_t);

    int ssize = 0;
    for (i = 0; i < num; i++) {
        switch(op[i].type) {
            case 0: list_push_front(lst, &op[i]); break;
            case 1: list_push_back(lst, &op[i]); break;
            case 2: if(!list_empty(lst))list_pop_front(lst); break;
            case 3: if(!list_empty(lst))list_pop_back(lst); break;
            case 4: list_insert(lst, iter_next_n(list_begin(lst), op[i].num), &op[i]);break;
            case 5: if(!list_empty(lst))
                list_erase(lst, iter_next_n(list_begin(lst), op[i].num));break;
        }
    }

    delta = get_time() - delta;
    printf("C List Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    //test c++ list
    delta = get_time();
    list<op_t> slist;
    for (i = 0; i < num; i++) {
        switch(op[i].type) {
            case 0: slist.push_front(op[i]); break;
            case 1: slist.push_back(op[i]); break;
            case 2: if(!slist.empty())slist.pop_front(); break;
            case 3: if(!slist.empty())slist.pop_back(); break;
            case 4: {
                list<op_t>::iterator it = slist.begin();
                advance(it, op[i].num);
                slist.insert(it, op[i]);
            }
            break;
            case 5: {
                list<op_t>::iterator it = slist.begin();
                advance(it, op[i].num);
                slist.erase(it);
            }
            break;
        }
    }
    delta = get_time() - delta;
    printf("STL List Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    fn_free(op);

    // check
    ck_assert_int_eq(list_size(lst), slist.size());
    iterator_t it0 = list_begin(lst);
    list<op_t>::iterator it1 = slist.begin();
    for (;iter_equal(list_end(lst), it0) && it1 != slist.end();
        it0 = iter_next(it0), it1++) {
        op_t opp;
        iter_get_value(it0, &opp);
        ck_assert_int_eq(opp.type, (*it1).type);
        ck_assert_int_eq(opp.num, (*it1).num);
    }

    list_delete(lst);

    // read time
    list_t *lst2 = list_new(int);
    list<int> sslist;
    list<int>::iterator it2;

    int pnum = 1000000;
    int *pnums = (int*)fn_malloc(sizeof(int) * pnum);
    int *xnums = (int*)fn_malloc(sizeof(int) * pnum);
    int *ynums = (int*)fn_malloc(sizeof(int) * pnum);

    for (i = 0; i < pnum; i++) pnums[i] = rand() % 1000;

    delta = get_time();
    for (i = 0; i < pnum; i++) {
        list_push_back(lst2, pnums[i]);
    }

    int ind = 0;
    for (it0 = list_begin(lst2); !iter_equal(list_end(lst2), it0); it0 = iter_next(it0)) {
        iter_get_value(it0, &xnums[ind++]);
    }
    delta = get_time() - delta;
    printf("C List Read Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);

    delta = get_time();
    for (i = 0; i < pnum; i++) {
        sslist.push_back(pnums[i]);
    }

    ind = 0;
    for (it2 = sslist.begin(); it2 != sslist.end(); it2++) {
        ynums[ind++] = *it2;
    }
    delta = get_time() - delta;
    printf("STL List Read Runtime: %d.%ds\n", (int)delta/US_ONE_SEC, (int)delta%US_ONE_SEC);
    // for (i = 0 ;i < pnum; i++) {
    //     ck_assert_int_eq(xnums[i], ynums[i]);
    // }
    list_delete(lst2);

    fn_free(pnums);
    fn_free(xnums);
    fn_free(ynums);
    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST

START_TEST(test_listop_iterator)
{
    list_t *lst;
    int i = 0;
    lst = list_new(int);
    ck_assert_int_eq(list_empty(lst), true);

    for (i = 0;i < 10; i++) {
        list_push_back(lst, i);
    }

    int aft_erase[] =  {-1, 1, 2, 4, 5, 6, 7, 8, 9};
    int aft_insert[] = {-1, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    iterator_t it0, it1, it2;
    int val1, val2;
    _ITER_INIT(it1);

    ck_assert_int_eq(_iter_is_vaild(it1), false);
    it1 = list_begin(lst);
    ck_assert_int_eq(_iter_is_vaild(it1), true);
    iter_get_value(it1, &val1);
    ck_assert_int_eq(val1, 0);

    it2 = list_end(lst);
    ck_assert_int_eq(iter_distance(it1, it2), 10);
    ck_assert_int_eq(iter_distance(it2, it1), -10);

    it0 = list_begin(lst);
    ck_assert_int_eq(iter_equal(it0, it1), true);
    ck_assert_int_eq(iter_equal(it0, it2), false);

    int ind = 0, value;
    for (it0 = list_begin(lst); !iter_equal(it0, list_end(lst));
        it0 = iter_next(it0)) {
        iter_get_value(it0, &value);
        ck_assert_int_eq(value, ind);
        ck_assert_int_eq(*(int*)iter_get_pointer(it0), ind);
        ind ++;
    }

    it0 = iter_next_n(list_begin(lst), 5);
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
    iter_set_value(list_begin(lst), &value);
    ck_assert_int_eq(*(int*)list_front(lst), -1);

    list_erase(lst, iter_next_n(list_begin(lst), 3));

    ind = 0;
    for (it0 = list_begin(lst); !iter_equal(it0, list_end(lst));
        it0 = iter_next(it0)) {
        iter_get_value(it0, &value);
        ck_assert_int_eq(value, aft_erase[ind]);
        ind++;
    }
    list_insert(lst, iter_next_n(list_begin(lst), 3), 3);

    ind = 0;
    for (it0 = list_begin(lst); !iter_equal(it0, list_end(lst));
        it0 = iter_next(it0)) {
        iter_get_value(it0, &value);
        ck_assert_int_eq(value, aft_insert[ind]);
        ind++;
    }

    list_delete(lst);
    ck_assert_int_eq(fn_memdbg_get_recnum(), 0);
}
END_TEST



Suite *clist_test_suite() {
    Suite *s = suite_create("=== CList Test ===");
    TCase *tc_core = tcase_create("TC core");
    tcase_set_timeout(tc_core, 1000);

    tcase_add_test(tc_core, test_newlist);
    tcase_add_test(tc_core, test_listop_cbuiltin);
    tcase_add_test(tc_core, test_listop_userdef);
    tcase_add_test(tc_core, test_listop_iterator);
#ifdef ENEFFTEST
    tcase_add_test(tc_core, test_listeffec);
#endif
    suite_add_tcase(s, tc_core);
    return s;
}