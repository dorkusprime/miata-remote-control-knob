# Workspace and access

## Verified setup

On 2026-10-26, this working session successfully:

- Created files in the local checkout.
- Authenticated to GitHub using the existing GitHub CLI/keychain setup.
- Cloned and pushed over SSH.
- Created and committed on main, with origin/main tracking.
- Verified the GitHub main branch matched the local commit and the working tree was clean.

Repository: https://github.com/dorkusprime/miata-remote-control-knob

The repository was already public when supplied. Keep credentials and unrelated personal files out of it. No additional ChatGPT GitHub plugin was necessary for this workflow.

## How access fits together

These are separate capabilities:

1. Local folder access permits reading/editing files in the checkout.
2. Git metadata writes permit staging and committing.
3. GitHub authentication identifies the account and authorizes repository operations.
4. Network access permits contacting GitHub over SSH/API.

All four worked in this session. Connecting a GitHub plugin is not required when the existing CLI route works, and it does not by itself grant access to arbitrary local folders.

For future sessions, open/select this checkout as the working folder and ask the agent to read AGENTS.md and README.md. If access fails, diagnose the specific failed operation rather than reconfiguring everything or copying credentials into chat.

## Routine checks

Run in this repository:

```sh
git status --short --branch
git remote -v
gh auth status
```

Before building firmware, read docs/tasks.md. No toolchain has been installed and no hardware has been flashed by this setup.

This checkout is inside the existing Google Drive-backed project folder. Use one working copy at a time; GitHub is the source-control remote. If simultaneous editing/sync conflicts become a problem, move the active checkout to an approved non-synced local project folder rather than maintaining two competing working copies.
