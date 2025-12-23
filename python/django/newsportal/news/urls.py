from django.urls import path

from . import views

urlpatterns = [
    path("", views.ArticleListView.as_view(), name="home"),
    path("articles/", views.ArticleListView.as_view(), name="article-list"),
    path("article/new/", views.ArticleCreateView.as_view(), name="article-create"),
    path(
        "article/<slug:slug>/", views.ArticleDetailView.as_view(), name="article-detail"
    ),
    path(
        "article/<slug:slug>/update/",
        views.ArticleUpdateView.as_view(),
        name="article-update",
    ),
    path(
        "article/<slug:slug>/delete/",
        views.ArticleDeleteView.as_view(),
        name="article-delete",
    ),
]
