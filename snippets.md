# snippets

Bits of code I write for various purposes.

## Picking from a weighted set

```
/* picks a random slot from a weighted table */
int pick() {
	unsigned weights[] = { 5, 10, 200, 40, 10 }, total, i, n;
	for (total = i = 0; i < 5; i++) total += weight[i];
	n = rand() % total;
	for (i = 0; i < 5 && n > weight[i]; i++) n -= weight[i];
	return i;
}
```
