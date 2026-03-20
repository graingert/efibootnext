# CLAUDE.md

## Release process

1. Update `debian/changelog` with the new version and changes.
2. Commit all changes.
3. `git tag v<version>`
4. `git push origin <branch> && git push origin v<version>`
5. `dpkg-buildpackage -us -uc -b`
6. `gh release create v<version> ../efibootnext_<version>_all.deb --title "v<version>" --notes "<summary>"`
