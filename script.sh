#!/usr/bin/env bash

# Define the list of branch names
branches=(
  "ecd/backtracking/all-sizes-with-symmetry-and-pruning/bfs-order"
  "ecd/backtracking/all-sizes-with-symmetry-and-pruning/heuristic/dsatur/loop"
  "ecd/backtracking/all-sizes-with-symmetry-and-pruning/heuristic/dsatur/multiset"
  "ecd/backtracking/all-sizes-with-symmetry-and-pruning/heuristic/dsatur/set"
  "ecd/backtracking/all-sizes-with-symmetry-and-pruning/heuristic/dsatur/vector"
  "ecd/backtracking/all-sizes-with-symmetry-and-pruning/random-order"
  "ecd/backtracking/target-size/bfs-order"
)

# Paths to the two repositories
BA_GRAPH_REPO=~/Matfyz/RP/OffRepo/ba-graph
REPORTZIMA_REPO=~/Matfyz/RP/OffRepo/ReportZima

for branch in "${branches[@]}"; do
  echo "============================"
  echo "Processing branch: $branch"
  echo "============================"

  # Step 1: Checkout the branch in ba-graph
  cd "$BA_GRAPH_REPO" || exit 1
  git checkout "$branch" || {
    echo "Error: Could not checkout branch '$branch' in ba-graph."
    exit 1
  }

  # Step 2: Create and switch to the same branch in ReportZima
  cd "$REPORTZIMA_REPO" || exit 1

  # (Optional) If you want to ensure you branch from 'main', do:
  #   git checkout main
  #   git pull origin main
  
  # Create the branch (force if it exists locally, you may want to remove --force if you don't want that)
  git checkout -b "$branch" 2>/dev/null || git checkout "$branch"

  # Step 3: Copy the file from ba-graph
  cp ../ba-graph/include/invariants/ecd.hpp . || {
    echo "Error: Could not copy 'ecd.hpp'."
    exit 1
  }

  # Step 4: Commit and push
  git add --all
  git commit -m "Add branch ecd"  # You can customize this message
  git push --set-upstream origin "$branch"
done

