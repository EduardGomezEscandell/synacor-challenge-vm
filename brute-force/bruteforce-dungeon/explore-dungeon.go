package main

import (
	"bytes"
	"fmt"
	"log"
	"math/rand"
	"os"
	"os/exec"
	"regexp"
	"strconv"
	"strings"
	"time"

	"golang.org/x/exp/slices"
)

const duration = 3 * time.Second

func main() {
	const N = 100
	ch := make(chan result, N)

	for i := 0; i < N; i++ {
		seed := rand.Int63()
		go func() {
			seed := seed
			lineCount, codes, err := explore(seed)
			ch <- result{seed, lineCount, codes, err}
		}()
	}

	results := make([]result, 0, N)
	for i := 0; i < N; i++ {
		results = append(results, <-ch)
	}
	close(ch)

	slices.SortFunc(results, func(a, b result) int {
		if len(a.codes) > len(b.codes) {
			return -1
		}
		if len(a.codes) < len(b.codes) {
			return 1
		}
		if a.execCount > b.execCount {
			return -1
		}
		if a.execCount < b.execCount {
			return 1
		}
		return 0
	})

	for _, r := range results {
		if r.err != nil {
			log.Printf("Seed %d had an error: %v", r.seed, r.err)
			continue
		}
		log.Printf("Seed %d executed %d bytes of code and found codes %s\n", r.seed, r.execCount, r.codes)
	}
}

type result struct {
	seed      int64
	execCount int
	codes     []string
	err       error
}

func explore(seed int64) (execCount int, codes []string, err error) {
	src := rand.NewSource(seed)

	var outBuffer, errBuffer bytes.Buffer

	cmd := exec.Command("../build/Release/vm/cmd/coverage", "../docs/spec/challenge")
	cmd.Stdout = &outBuffer
	cmd.Stderr = &errBuffer

	stdin, err := cmd.StdinPipe()
	if err != nil {
		return execCount, codes, err
	}
	defer stdin.Close()

	stepsf, err := os.Create(fmt.Sprintf("results/%d.steps", seed))
	if err != nil {
		return execCount, codes, err
	}
	defer stepsf.Close()

	start := `take tablet
use tablet
doorway
north
north
bridge
continue
down
east
take empty lantern
west
west
passage
ladder
west
south
north
take can
look can
use can
west
ladder
darkness
use lantern
continue
west
west
west
west
north
take red coin
north
west
take blue coin
up
take shiny coin
down
east
east
take concave coin
down
take corroded coin
up
west
use blue coin
use red coin
use shiny coin
use concave coin
use corroded coin
north
take teleporter
use teleporter`

	fmt.Fprintln(stdin, start)
	fmt.Fprintln(stepsf, start)

	if err := cmd.Start(); err != nil {
		return execCount, codes, fmt.Errorf("Could not start: %v", err)
	}

	end := make(chan struct{})
	time.AfterFunc(duration, func() {
		cmd.Process.Signal(os.Interrupt)
		close(end)
	})

loop:
	for {
		select {
		case <-end:
			break loop
		case <-time.After(time.Millisecond):
		}

		answ, ok := select_answer(src, outBuffer.String())
		if ok {
			fmt.Fprintln(stepsf, answ)
			fmt.Fprintln(stdin, answ)
		}
	}

	time.Sleep(100 * time.Millisecond)

	if err := os.WriteFile(fmt.Sprintf("results/%d", seed), outBuffer.Bytes(), 0644); err != nil {
		log.Printf("Error: %v\n", err)
	}

	if !strings.Contains(errBuffer.String(), "-------------") {
		return execCount, codes, fmt.Errorf("Output buffer does not contain summary: %v", err)
	}

	pattern := regexp.MustCompile(`Covered ([0-9]+) addresses`)
	r := pattern.FindStringSubmatch(errBuffer.String())
	if len(r) != 2 {
		return execCount, codes, fmt.Errorf("Could not parser stderr: %s", errBuffer.String())
	}
	execCount, err = strconv.Atoi(r[1])

	pattern = regexp.MustCompile(`[^\w]([a-zA-Z]{12})[^\w]`)
	match := pattern.FindAllStringSubmatch(outBuffer.String(), -1)
	if len(match) != 0 {
		d := map[string]int{
			// Known collisions
			"dramatically": 1,
			"instructions": 1,
			"Headquarters": 1,
			"Introduction": 1,
			"fundamentals": 1,
			"mathematical": 1,
			"interactions": 1,
			"hypothetical": 1,
			"destinations": 1,
			"confirmation": 1,
		}
		// Finding unique entries
		for _, m := range match {
			candidate := m[1]
			if d[candidate] != 0 {
				continue
			}
			codes = append(codes, candidate)
			d[candidate] += 1
		}
	}

	cmd.Wait()

	return execCount, codes, err
}

var relevantLines = regexp.MustCompile("\n- ([^\n]+)\n")

// Biased towards 'go' on purpose
var verbs = []string{"look", "inv", "take", "drop", "use", "go", "go", "go", "go", "go", "go", "go"}

func select_answer(src rand.Source, s string) (string, bool) {
	matches := relevantLines.FindAllStringSubmatch(s, -1)
	if len(matches) == 0 {
		return "", false
	}

	vid := src.Int63() % int64(len(verbs))
	tgt := src.Int63() % int64(len(matches))

	return fmt.Sprintf("%s %s", verbs[vid], matches[tgt][1]), true
}
