#include <bits/stdc++.h>
#include <ext/pb_ds/tree_policy.hpp>
#include <functional>

#define int long long
#define all(a) a.begin(), a.end()
#define pb push_back
#define eb emplace_back
#define testcases \
    int t;        \
    cin >> t;     \
    while (t--)
#define loop(a, b) for (int i = a; i < b; i++)
#define sumvec(vec) accumulate(vec.begin(), vec.end(), (int)0)
#define sumarr(arr) accumulate(arr, arr + sizeof(arr) / sizeof(arr[0]), (int)0)
#define outarr(arr)                                          \
    for (int i = 0; i < (sizeof(arr) / sizeof(arr[0])); i++) \
        cout << arr[i] << endl;
#define outvec(v)                      \
    for (int i = 0; i < v.size(); i++) \
        cout << v[i] << endl;
using namespace std;
using namespace __gnu_pbds;

signed main()
{
    std::string str = "Hello, how are you doing today?";
    std::vector<std::string> str_tok;
    std::istringstream iss(str);
    std::copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), std::back_inserter(str_tok));

    for (auto s : str_tok)
    {
        std::cout << s << std::endl;
    }

    return 0;
}

/*
"Hello,"
"how"
"are"
"you"
"doing"
"today?"

*/