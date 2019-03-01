git filter-branch --commit-filter \
'if [ "$GIT_AUTHOR_NAME" = "Thetoto" ]; then \
export GIT_AUTHOR_NAME="Thomas Lupin";\
export GIT_AUTHOR_EMAIL=thomas.lupin@epita.fr;\
export GIT_COMMITTER_NAME="Thomas Lupin";\
export GIT_COMMITTER_EMAIL=thomas.lupin@epita.fr;\
fi;\
git commit-tree "$@"'
