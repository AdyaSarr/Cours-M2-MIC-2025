package main

import (
	"slices"
)

const TAILLE = 1000

func erathostene_algo(n int) []int {
	if n > TAILLE {
		n = TAILLE
	}
	a := make([]int, n)
	for i := 1; i < n; i++ {
		a[i] = i
	}

	for i := 2; i < n; i++ {
		for j := i + 1; j < n; j++ {
			if j%i == 0 {
				a[j] = 0
			}
		}
	}
	prime_numbers := make([]int, 0)
	j := 0

	for i := 2; i < n; i++ {
		if a[i] != 0 {
			prime_numbers = append(prime_numbers, a[i])
			j++
		}
	}
	return prime_numbers
}

func erathostene_algo_efficace(n int) []int {
	if n > TAILLE {
		n = TAILLE
	}
	a := make([]int, n-1)
	for i := 0; i < len(a); i++ {
		a[i] = i + 2
	}
	for i := 0; i < len(a); i++ {
		for j := i + 1; j < len(a); {
			if a[j]%a[i] == 0 {
				a = slices.Delete(a, j, j+1)
			} else {
				j++
			}
		}
	}
	return a
}
