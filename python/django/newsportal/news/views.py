from django.contrib import messages
from django.contrib.auth.mixins import LoginRequiredMixin, UserPassesTestMixin
from django.db.models import Q
from django.shortcuts import redirect
from django.urls import reverse, reverse_lazy
from django.utils.text import slugify
from django.views.generic import (
    CreateView,
    DeleteView,
    DetailView,
    ListView,
    UpdateView,
)

from .forms import ArticleSearchForm, CommentForm
from .models import Article, Category


class ArticleListView(ListView):
    model = Article
    template_name = "news/article_list.html"
    context_object_name = "articles"
    paginate_by = 10

    def get_queryset(self):
        queryset = Article.objects.filter(status="published")

        search_query = self.request.GET.get("q")
        if search_query:
            queryset = queryset.filter(
                Q(title__icontains=search_query)
                | Q(content__icontains=search_query)
                | Q(summary__icontains=search_query)
            )

        category_slug = self.request.GET.get("category")
        if category_slug:
            queryset = queryset.filter(category__slug=category_slug)

        if self.request.user.is_authenticated:
            user_drafts = Article.objects.filter(
                author=self.request.user, status="draft"
            )
            queryset = queryset | user_drafts

        return queryset.distinct().order_by("-created_at")

    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)
        context["search_form"] = ArticleSearchForm(self.request.GET or None)
        context["categories"] = Category.objects.all()
        return context


class ArticleDetailView(DetailView):
    model = Article
    template_name = "news/article_detail.html"
    context_object_name = "article"

    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)
        context["comment_form"] = CommentForm()
        context["comments"] = self.object.comments.filter(is_active=True)
        return context

    def post(self, request, *args, **kwargs):
        self.object = self.get_object()
        if request.user.is_authenticated:
            form = CommentForm(request.POST)
            if form.is_valid():
                comment = form.save(commit=False)
                comment.article = self.object
                comment.author = request.user
                comment.save()
                messages.success(request, "Комментарий добавлен!")
                return redirect("article-detail", slug=self.object.slug)
        return self.get(request, *args, **kwargs)


class ArticleCreateView(LoginRequiredMixin, CreateView):
    model = Article
    template_name = "news/article_form.html"
    fields = ["title", "summary", "content", "image", "category", "tags", "status"]
    success_url = reverse_lazy("article-list")

    def form_valid(self, form):
        form.instance.author = self.request.user

        if not form.instance.slug:
            form.instance.slug = slugify(form.instance.title)
        return super().form_valid(form)


class ArticleUpdateView(LoginRequiredMixin, UserPassesTestMixin, UpdateView):
    model = Article
    template_name = "news/article_form.html"
    fields = ["title", "summary", "content", "image", "category", "tags", "status"]

    def test_func(self):
        article = self.get_object()
        return self.request.user == article.author

    def get_success_url(self):
        messages.success(self.request, "Статья успешно обновлена!")
        return reverse("article-detail", kwargs={"slug": self.object.slug})

    def form_valid(self, form):
        return super().form_valid(form)


class ArticleDeleteView(LoginRequiredMixin, UserPassesTestMixin, DeleteView):
    model = Article
    template_name = "news/article_confirm_delete.html"
    success_url = reverse_lazy("article-list")

    def test_func(self):
        article = self.get_object()
        return self.request.user == article.author

    def delete(self, request, *args, **kwargs):
        messages.success(request, "Статья успешно удалена!")
        return super().delete(request, *args, **kwargs)
